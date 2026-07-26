/*
 * Warlock Legendaries
 *
 * Fifteen warlock-only legendaries (entries 900001..900015). SQL defines the items,
 * stats, and simple existing-spell effects (Blink / Anti-Magic Shell / Dispersion /
 * Death Grip / Mirror Image on the cross-class trinkets, Corruption / Immolate /
 * Shadow Nova on the proc weapons). This file wires the four bits SQL can't cover:
 *
 *   * Drop pipeline. Any qualifying kill by (or credited to) a warlock rolls once
 *     for a legendary; on success one is minted and delivered by mail from the
 *     "voidcaller" NPC entry — see the "The Void" mail.
 *   * On-use custom effects: Voidcaller's Sigil (full-restore or resummon your last
 *     demon), Heart of Kanrethad (temporary +damage/-mitigation aura on your
 *     demon), Doomstaff of Ner'zhul (temporary Doomguard for 45s).
 *   * Passive equip effect: Ring of the Voidsworn adds flat stamina to the active
 *     demon while equipped, and stays synced across summon / (un)equip.
 *   * The Signet of the Feltouched and Fel Splinter "extra Demonic Empowerment"
 *     hook lives in warlock_demonic_empowerment.cpp so that all Demonic Empowerment
 *     bookkeeping happens in one place — this file only exports the item IDs it
 *     needs (see warlock_legendaries.h).
 */

#include "warlock_legendaries.h"

#include "Chat.h"
#include "Config.h"
#include "DataMap.h"
#include "DatabaseEnv.h"
#include "Formulas.h"
#include "Item.h"
#include "ItemScript.h"
#include "Log.h"
#include "Mail.h"
#include "ObjectMgr.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerScript.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "StringFormat.h"
#include "TemporarySummon.h"
#include "Timer.h"
#include "WorldSession.h"

#include <array>
#include <cmath>
#include <unordered_map>

using namespace WarlockLegendaries;

namespace
{
    // Full pool of legendaries eligible to drop.
    constexpr std::array<uint32, 15> LEGENDARY_POOL = {
        ITEM_RING_MALCHEZAAR_PASSAGE,
        ITEM_SIGNET_OF_THE_FELTOUCHED,
        ITEM_RING_OF_THE_VOIDSWORN,
        ITEM_FROSTMOURNE_SHARD,
        ITEM_KELTHUZAD_PHYLACTERY,
        ITEM_VOIDCALLER_SIGIL,
        ITEM_HEART_OF_KANRETHAD,
        ITEM_DOOMSTAFF_OF_NER_ZHUL,
        ITEM_SACROPHILE_BLADE,
        ITEM_FEL_IRON_SKEWER,
        ITEM_MANNOROTHS_FEMUR,
        ITEM_VOID_GRIP_KIL_JAEDEN,
        ITEM_MIRROR_TWIN_EMPERORS,
        ITEM_FEL_SPLINTER,
        ITEM_KANRETHADS_REACH
    };

    // Doomguard NPC entry used both by the on-use temporary summon (Doomstaff)
    // and as the default mail sender ("A Voidcaller's Whisper").
    // Named DOOMGUARD_ENTRY because PetDefines.h already declares an unscoped
    // NPC_DOOMGUARD enumerator with the same value, which makes the plain name
    // ambiguous in this translation unit.
    constexpr uint32 DOOMGUARD_ENTRY = 11859;

    // Per-item cooldown durations (ms) for our custom on-use items. We track them
    // via Player CustomData rather than Player::AddSpellCooldown because the items
    // ship with spellid_1 = 0 (no real Spell.dbc entry to piggy-back on); shoving
    // a synthetic id into m_spellCooldowns would spam ERROR logs at load time.
    // CustomData dies with the WorldSession, so cooldowns reset on logout — an
    // acceptable trade for zero-noise persistence.
    constexpr uint32 CD_VOIDCALLER_SIGIL_MS      = 180u * 1000u;
    constexpr uint32 CD_HEART_OF_KANRETHAD_MS    = 120u * 1000u;
    constexpr uint32 CD_DOOMSTAFF_OF_NER_ZHUL_MS = 300u * 1000u;

    class LegendaryCooldowns : public DataMap::Base
    {
    public:
        std::unordered_map<uint32 /*item entry*/, uint32 /*ready-at MSTime*/> readyAtMs;
    };

    constexpr char const* COOLDOWNS_KEY = "WarlockLegendaries.Cooldowns";

    // True if `itemEntry` is still on cooldown for `player`. Called before running
    // any on-use side effects so a failed on-CD check leaves state untouched.
    bool IsOnLegendaryCooldown(Player* player, uint32 itemEntry)
    {
        auto* cds = player->CustomData.GetDefault<LegendaryCooldowns>(COOLDOWNS_KEY);
        auto it = cds->readyAtMs.find(itemEntry);
        return it != cds->readyAtMs.end() && it->second > getMSTime();
    }

    void StartLegendaryCooldown(Player* player, uint32 itemEntry, uint32 durationMs)
    {
        auto* cds = player->CustomData.GetDefault<LegendaryCooldowns>(COOLDOWNS_KEY);
        cds->readyAtMs[itemEntry] = getMSTime() + durationMs;
    }

    // Existing spell used by the Heart of Kanrethad to "berserk" the demon.
    // Bestial Wrath (19574) applies a 18s buff to the target pet: +50% damage,
    // -25% damage taken, CC-immunity. Thematically identical to what we want,
    // works out-of-the-box for any demon we throw at it, and gives us free
    // client-side art (the wrath aura visual).
    constexpr uint32 SPELL_BESTIAL_WRATH_ANALOG = 19574;

    // Temp Doomguard duration for Doomstaff of Ner'zhul.
    constexpr uint32 DOOMSTAFF_TEMP_DURATION_MS = 45u * 1000u;

    // Per-pet record: how much Voidsworn stamina is currently applied to this pet
    // object. Kept on the pet's CustomData so it dies with the pet — no GUID-reuse
    // or cleanup hazards, exactly the same pattern as warlock_demonic_empowerment.cpp.
    // We store the applied amount (not a bool) so a config reload mid-session can't
    // strand a differently-sized modifier on the pet.
    class VoidswornPetState : public DataMap::Base
    {
    public:
        float applied = 0.0f;
    };

    constexpr char const* PET_STATE_KEY = "WarlockLegendaries.Voidsworn";

    bool IsEnabled()
    {
        return sConfigMgr->GetOption<bool>(CONFIG_ENABLED, true);
    }

    bool IsWarlock(Player const* player)
    {
        return player && player->IsClass(CLASS_WARLOCK, CLASS_CONTEXT_PET);
    }

    void SendMessageIfOnline(Player* player, std::string const& msg)
    {
        if (WorldSession* session = player->GetSession())
            ChatHandler(session).SendSysMessage(msg);
    }

    int32 VoidswornStamBonus()
    {
        return sConfigMgr->GetOption<int32>(CONFIG_VOIDSWORN_STAM_BONUS, 250);
    }

    // Applies (or removes) the Voidsworn stamina bump to a pet, updating the
    // per-pet applied amount so we never double-apply or double-remove. Flat
    // TOTAL_VALUE modifiers survive Guardian::InitStatsForLevel (it only rewrites
    // base/create stats), so the recorded amount stays accurate across re-inits.
    void SyncVoidswornOnPet(Player* owner, Unit* pet)
    {
        if (!owner || !pet)
            return;

        float want = owner->HasItemOrGemWithIdEquipped(ITEM_RING_OF_THE_VOIDSWORN, 1)
            ? float(VoidswornStamBonus()) : 0.0f;
        if (want < 0.0f)
            want = 0.0f;

        auto* state = pet->CustomData.GetDefault<VoidswornPetState>(PET_STATE_KEY);
        if (want == state->applied)
            return;

        if (state->applied != 0.0f)
            pet->HandleStatFlatModifier(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE, state->applied, false);
        if (want != 0.0f)
            pet->HandleStatFlatModifier(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE, want, true);

        pet->UpdateAllStats();
        state->applied = want;
    }

    // Kill was "qualifying" (grants XP/rep). Mirrors the check in warlock_demonic_empowerment.cpp
    // so drop chance can't be farmed off grey mobs or on player kills.
    bool IsQualifyingCreatureKill(Player const* killer, Creature const* victim)
    {
        if (!killer || !victim)
            return false;
        if (victim->IsControlledByPlayer())
            return false;
        return victim->GetLevel() > Acore::XP::GetGrayLevel(killer->GetLevel());
    }

    bool CreatureRankQualifies(Creature const* victim)
    {
        if (!victim)
            return false;

        CreatureTemplate const* ct = victim->GetCreatureTemplate();
        if (!ct)
            return false;

        // ct->rank: 0=normal, 1=elite, 2=rare-elite, 3=world-boss, 4=rare.
        // With minRank=1 (default), we drop for elite / rare-elite / world-boss / rare —
        // i.e. anything that isn't a common trash mob. minRank<=0 disables the gate.
        int32 minRank = sConfigMgr->GetOption<int32>(CONFIG_MIN_CREATURE_RANK, 1);
        if (minRank <= 0)
            return true;

        return int32(ct->rank) >= minRank && int32(ct->rank) > 0;
    }

    // Mails the picked legendary to `receiver`. Sender is a configurable NPC
    // entry (defaults to a Doomguard, 11859) so the mail bar shows a demon name.
    void SendLegendaryMail(Player* receiver, uint32 itemId)
    {
        if (!receiver)
            return;

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
        {
            LOG_ERROR("scripts.custom", "WarlockLegendaries: itemId {} has no template; skipping mail.", itemId);
            return;
        }

        Item* mailed = Item::CreateItem(itemId, 1);
        if (!mailed)
        {
            LOG_ERROR("scripts.custom", "WarlockLegendaries: Item::CreateItem failed for {}", itemId);
            return;
        }

        uint32 senderEntry = uint32(sConfigMgr->GetOption<int32>(CONFIG_MAIL_SENDER_ENTRY, int32(DOOMGUARD_ENTRY)));

        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        mailed->SaveToDB(trans);

        MailDraft("A Whisper from the Void",
                  "The Legion stirs. Something of terrible potency has drifted into your hands.\n\n"
                  "Guard it well, warlock — the demons remember every promise.")
            .AddItem(mailed)
            .SendMailTo(trans, MailReceiver(receiver), MailSender(MAIL_CREATURE, senderEntry));

        CharacterDatabase.CommitTransaction(trans);

        SendMessageIfOnline(receiver, Acore::StringFormat(
            "|cffff8000A Voidcaller's Whisper:|r |cffff8000|Hitem:{}:0:0:0:0:0:0:0:0|h[{}]|h|r has been mailed to you.",
            itemId, proto->Name1));
    }

    // Roll for a legendary drop off `victim` for `killer` (or the killer's owner).
    void HandleLegendaryDropRoll(Player* killer, Creature* victim)
    {
        if (!IsEnabled() || !IsWarlock(killer))
            return;
        if (!IsQualifyingCreatureKill(killer, victim))
            return;

        int32 minLevel = sConfigMgr->GetOption<int32>(CONFIG_MIN_CREATURE_LEVEL, 60);
        if (int32(victim->GetLevel()) < minLevel)
            return;

        if (!CreatureRankQualifies(victim))
            return;

        float chance = sConfigMgr->GetOption<float>(CONFIG_DROP_CHANCE_PERCENT, 0.5f);
        if (chance <= 0.0f || !roll_chance_f(chance))
            return;

        uint32 pick = LEGENDARY_POOL[urand(0u, uint32(LEGENDARY_POOL.size()) - 1u)];
        SendLegendaryMail(killer, pick);
    }
}

// -----------------------------------------------------------------------------
// ItemScript: Voidcaller's Sigil (900006)
// -----------------------------------------------------------------------------

class item_voidcaller_sigil : public ItemScript
{
public:
    item_voidcaller_sigil() : ItemScript("item_voidcaller_sigil") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!player)
            return true;

        // We always handle the use ourselves (return true), so the client is never
        // sent a cast — release the item from its pending/grey state explicitly.
        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);

        if (IsOnLegendaryCooldown(player, ITEM_VOIDCALLER_SIGIL))
        {
            SendMessageIfOnline(player, "|cffff8000The Voidcaller's Sigil still cools.|r");
            return true;
        }

        Pet* pet = player->GetPet();
        if (pet)
        {
            if (!pet->IsAlive())
            {
                pet->setDeathState(DeathState::Alive);
                pet->SetHealth(pet->GetMaxHealth());
            }
            else
            {
                pet->SetFullHealth();
            }
            pet->SetPower(pet->getPowerType(), pet->GetMaxPower(pet->getPowerType()));
            SendMessageIfOnline(player, "|cff9370dbThe void yields. Your demon is restored.|r");
        }
        else
        {
            uint32 lastPetSpell = player->GetLastPetSpell();
            if (!lastPetSpell)
            {
                SendMessageIfOnline(player, "|cffff8000No bound demon remembers you.|r");
                return true;
            }
            player->CastSpell(player, lastPetSpell, true);
            SendMessageIfOnline(player, "|cff9370dbThe void yields. Your demon returns to your side.|r");
        }

        StartLegendaryCooldown(player, ITEM_VOIDCALLER_SIGIL, CD_VOIDCALLER_SIGIL_MS);
        return true;
    }
};

// -----------------------------------------------------------------------------
// ItemScript: Heart of Kanrethad (900007)
// -----------------------------------------------------------------------------

class item_heart_of_kanrethad : public ItemScript
{
public:
    item_heart_of_kanrethad() : ItemScript("item_heart_of_kanrethad") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!player)
            return true;

        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);

        if (IsOnLegendaryCooldown(player, ITEM_HEART_OF_KANRETHAD))
        {
            SendMessageIfOnline(player, "|cffff8000The Heart of Kanrethad beats slowly.|r");
            return true;
        }

        Pet* pet = player->GetPet();
        if (!pet || !pet->IsAlive())
        {
            SendMessageIfOnline(player, "|cffff8000No living demon to empower.|r");
            return true;
        }

        // Bestial Wrath — +50% damage, -25% damage taken, CC-immunity, 18s.
        player->CastSpell(pet, SPELL_BESTIAL_WRATH_ANALOG, true);
        SendMessageIfOnline(player, "|cff9370dbFel-blood floods your demon.|r");

        StartLegendaryCooldown(player, ITEM_HEART_OF_KANRETHAD, CD_HEART_OF_KANRETHAD_MS);
        return true;
    }
};

// -----------------------------------------------------------------------------
// ItemScript: Doomstaff of Ner'zhul (900008)
// -----------------------------------------------------------------------------

class item_doomstaff_of_nerzhul : public ItemScript
{
public:
    item_doomstaff_of_nerzhul() : ItemScript("item_doomstaff_of_nerzhul") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        if (!player)
            return true;

        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);

        if (IsOnLegendaryCooldown(player, ITEM_DOOMSTAFF_OF_NER_ZHUL))
        {
            SendMessageIfOnline(player, "|cffff8000The Doomstaff's chorus is silent.|r");
            return true;
        }

        // The item's client-side spell (Inferno) is ground-targeted, so the player
        // aims where the Doomguard appears. Fall back to just in front of the player
        // if no destination came through.
        float ox = player->GetPositionX() + std::cos(player->GetOrientation()) * 2.5f;
        float oy = player->GetPositionY() + std::sin(player->GetOrientation()) * 2.5f;
        float oz = player->GetPositionZ();
        float oo = player->GetOrientation();
        if (targets.HasDst())
        {
            if (WorldLocation const* dest = targets.GetDstPos())
            {
                ox = dest->GetPositionX();
                oy = dest->GetPositionY();
                oz = dest->GetPositionZ();
            }
        }

        TempSummon* summon = player->SummonCreature(DOOMGUARD_ENTRY, ox, oy, oz, oo,
            TEMPSUMMON_TIMED_DESPAWN, DOOMSTAFF_TEMP_DURATION_MS);

        if (!summon)
        {
            SendMessageIfOnline(player, "|cffff8000The Doomstaff falters.|r");
            return true;
        }

        summon->SetOwnerGUID(player->GetGUID());
        summon->SetFaction(player->GetFaction());
        summon->SetLevel(player->GetLevel());

        if (Unit* target = player->GetSelectedUnit())
            if (summon->IsValidAttackTarget(target))
                summon->AI()->AttackStart(target);

        SendMessageIfOnline(player, "|cff9370dbA second doom answers your call.|r");

        StartLegendaryCooldown(player, ITEM_DOOMSTAFF_OF_NER_ZHUL, CD_DOOMSTAFF_OF_NER_ZHUL_MS);
        return true;
    }
};

// -----------------------------------------------------------------------------
// PlayerScript: legendary drop hook + Voidsworn ring passive sync.
// -----------------------------------------------------------------------------

class warlock_legendaries_playerscript : public PlayerScript
{
public:
    warlock_legendaries_playerscript() : PlayerScript(
        "warlock_legendaries_playerscript",
        {
            PLAYERHOOK_ON_CREATURE_KILL,
            PLAYERHOOK_ON_CREATURE_KILLED_BY_PET,
            PLAYERHOOK_ON_AFTER_GUARDIAN_INIT_STATS_FOR_LEVEL,
            PLAYERHOOK_ON_EQUIP,
            PLAYERHOOK_ON_UNEQUIP_ITEM
        }) { }

    void OnPlayerCreatureKill(Player* killer, Creature* killed) override
    {
        HandleLegendaryDropRoll(killer, killed);
    }

    void OnPlayerCreatureKilledByPet(Player* petOwner, Creature* killed) override
    {
        HandleLegendaryDropRoll(petOwner, killed);
    }

    void OnPlayerAfterGuardianInitStatsForLevel(Player* player, Guardian* guardian) override
    {
        if (!IsEnabled() || !IsWarlock(player) || !guardian)
            return;

        // Guardian::InitStatsForLevel fires on summon and on level-ups. It only
        // rewrites base stats — our flat modifier (and the recorded applied amount)
        // survives, so a plain re-sync is enough. Do NOT zero the state here or the
        // bonus would be stacked again on every re-init.
        SyncVoidswornOnPet(player, guardian);
    }

    void OnPlayerEquip(Player* player, Item* it, uint8 /*bag*/, uint8 /*slot*/, bool /*update*/) override
    {
        if (!IsEnabled() || !IsWarlock(player) || !it)
            return;
        if (it->GetEntry() != ITEM_RING_OF_THE_VOIDSWORN)
            return;

        if (Pet* pet = player->GetPet())
            SyncVoidswornOnPet(player, pet);
    }

    void OnPlayerUnequip(Player* player, Item* it) override
    {
        if (!IsEnabled() || !IsWarlock(player) || !it)
            return;
        if (it->GetEntry() != ITEM_RING_OF_THE_VOIDSWORN)
            return;

        if (Pet* pet = player->GetPet())
            SyncVoidswornOnPet(player, pet);
    }
};

// -----------------------------------------------------------------------------
// Registration
// -----------------------------------------------------------------------------

void AddSC_warlock_legendaries()
{
    new item_voidcaller_sigil();
    new item_heart_of_kanrethad();
    new item_doomstaff_of_nerzhul();
    new warlock_legendaries_playerscript();
}
