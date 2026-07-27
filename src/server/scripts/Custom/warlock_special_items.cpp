/*
 * Warlock Special Items — scripted mechanics for 18 signature items in the
 * 120-item warlock expansion (900018..900137). Remaining items are SQL-only.
 */

#include "warlock_special_items.h"

#include "Chat.h"
#include "CellImpl.h"
#include "Config.h"
#include "Creature.h"
#include "DataMap.h"
#include "Duration.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Item.h"
#include "ItemScript.h"
#include "ObjectAccessor.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerScript.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "StringFormat.h"
#include "TemporarySummon.h"
#include "Timer.h"
#include "UnitScript.h"
#include "WorldSession.h"

#include <array>
#include <cmath>
#include <unordered_map>

using namespace WarlockSpecialItems;

namespace
{
    constexpr std::array<uint32, 19> SCRIPTED_ITEMS = {
        ITEM_FEL_SPARK_SIGNET, ITEM_STARTERS_SOUL_PIN, ITEM_WORGEN_CALLERS_STAFF,
        ITEM_ABYSSAL_SOUL_TRINKET, ITEM_FELGUARD_CLEAVER, ITEM_DEATHS_HEAD_SOUL_PIN,
        ITEM_PRINCESS_SOUL_LOCKET, ITEM_IMMOLTHAR_MANAFEED, ITEM_DREADLORD_CLAW,
        ITEM_ARAN_EMBER_CLOAK, ITEM_DIMENSIUS_DUST, ITEM_MAGTHERIDON_CUBE,
        ITEM_FESTERGUT_PLAGUE, ITEM_BLOODSEAL_NETHERKURSE, ITEM_VOIDHEART,
        ITEM_SEAL_FIRST_NECROLYTE, ITEM_GRIMOIRE_EREDAR_TWINS, ITEM_SOULFLAME_LANTERN,
        ITEM_SIGNET_RESTLESS_VOID
    };

    // Pursuit of Justice r2 — +15% run (+mounted via core spell_linked_spell).
    // Applied from C++ so the item tooltip is not the paladin talent name.
    constexpr uint32 SPELL_RESTLESS_VOID_MS = 26023;

    // All Life Tap / Drain Life ranks (family-mask matching is unreliable here:
    // Life Tap is a dummy-effect spell, so we match rank ids explicitly).
    constexpr std::array<uint32, 8> LIFE_TAP_RANKS   = { 1454, 1455, 1456, 11687, 11688, 11689, 27222, 57946 };
    constexpr std::array<uint32, 9> DRAIN_LIFE_RANKS = { 689, 699, 709, 7651, 11699, 11700, 27219, 27220, 47857 };

    // Corruption ranks (Classic through WotLK max). Voidheart permanence/spread
    // must match every rank the player can actually cast.
    constexpr std::array<uint32, 10> CORRUPTION_RANKS = {
        172, 6222, 6223, 7648, 11671, 11672, 25311, 27216, 47812, 47813
    };
    constexpr uint32 SPELL_CORRUPTION_MAX    = 47813;
    constexpr uint32 SPELL_METAMORPHOSIS     = 47241;
    // NPC Shadow Nova — self-centered AoE. Do NOT use 42223 (Rain of Fire).
    constexpr uint32 SPELL_SHADOW_NOVA       = 32711;
    constexpr uint32 SPELL_SHADOWFLAME       = 47897;
    constexpr uint32 SPELL_BESTIAL_WRATH     = 19574;
    constexpr uint32 SPELL_IMMOLATE          = 47811; // WotLK max rank
    constexpr uint32 NPC_IMP                 = 416;

    constexpr uint32 CD_GENERIC_MS           = 120u * 1000u;
    constexpr uint32 CD_GRIMOIRE_MS          = 180u * 1000u;
    constexpr int32  DEATHS_HEAD_SP_PER_KILL = 5;
    constexpr int32  DEATHS_HEAD_SP_CAP      = 50; // 10 kills worth

    class SpecialCooldowns : public DataMap::Base
    {
    public:
        std::unordered_map<uint32, uint32> readyAtMs;
    };

    constexpr char const* CD_KEY = "WarlockSpecialItems.Cooldowns";

    class SpecialState : public DataMap::Base
    {
    public:
        uint32 bloodsealStacks     = 0;
        int32  bloodsealSpApplied  = 0;
        uint32 voidPressure        = 0;
        bool   voidNovaPending     = false;
        uint32 soulflameCharges    = 0;
        int32  soulflameSpApplied  = 0;
        ObjectGuid reapingTarget;
        uint32 reapingExpireMs     = 0;
        int32  felSparkSpApplied   = 0;
        int32  staffSpApplied      = 0;
        int32  dreadlordSpApplied  = 0;
        int32  dimensiusSpApplied  = 0;
        int32  festergutSpApplied  = 0;
        int32  deathsHeadSpApplied = 0;
    };

    constexpr char const* STATE_KEY = "WarlockSpecialItems.State";

    bool IsEnabled()
    {
        return sConfigMgr->GetOption<bool>(CONFIG_ENABLED, true);
    }

    bool IsWarlock(Player const* player)
    {
        return player && player->IsClass(CLASS_WARLOCK, CLASS_CONTEXT_PET);
    }

    bool IsLifeTapSpell(SpellInfo const* info)
    {
        if (!info)
            return false;
        for (uint32 id : LIFE_TAP_RANKS)
            if (info->Id == id)
                return true;
        return false;
    }

    bool IsDrainLifeSpell(SpellInfo const* info)
    {
        if (!info)
            return false;
        for (uint32 id : DRAIN_LIFE_RANKS)
            if (info->Id == id)
                return true;
        return false;
    }

    bool IsCorruptionSpell(SpellInfo const* info)
    {
        if (!info)
            return false;
        for (uint32 id : CORRUPTION_RANKS)
            if (info->Id == id)
                return true;
        return false;
    }

    Aura* FindPlayerCorruption(Unit* unit, ObjectGuid caster)
    {
        if (!unit)
            return nullptr;
        for (uint32 id : CORRUPTION_RANKS)
            if (Aura* aura = unit->GetAura(id, caster))
                return aura;
        return nullptr;
    }

    SpecialState* State(Player* player)
    {
        return player->CustomData.GetDefault<SpecialState>(STATE_KEY);
    }

    void Send(Player* player, std::string const& msg)
    {
        if (WorldSession* session = player->GetSession())
            ChatHandler(session).SendSysMessage(msg);
    }

    bool OnCooldown(Player* player, uint32 itemEntry)
    {
        auto* cds = player->CustomData.GetDefault<SpecialCooldowns>(CD_KEY);
        auto it = cds->readyAtMs.find(itemEntry);
        return it != cds->readyAtMs.end() && it->second > getMSTime();
    }

    void StartCooldown(Player* player, uint32 itemEntry, uint32 ms)
    {
        player->CustomData.GetDefault<SpecialCooldowns>(CD_KEY)->readyAtMs[itemEntry] = getMSTime() + ms;
    }

    void ClearBloodsealStacks(Player* player)
    {
        SpecialState* st = State(player);
        if (st->bloodsealSpApplied)
            player->ApplySpellPowerBonus(st->bloodsealSpApplied, false);
        st->bloodsealSpApplied = 0;
        st->bloodsealStacks = 0;
    }

    void SyncSoulflameSp(Player* player)
    {
        SpecialState* st = State(player);
        uint32 want = st->soulflameCharges / 2;
        if (want > 50)
            want = 50;
        if (int32(want) == st->soulflameSpApplied)
            return;
        if (st->soulflameSpApplied)
            player->ApplySpellPowerBonus(st->soulflameSpApplied, false);
        st->soulflameSpApplied = int32(want);
        if (want)
            player->ApplySpellPowerBonus(int32(want), true);
    }

    void SyncEquipPassives(Player* player)
    {
        if (!IsEnabled() || !IsWarlock(player))
            return;

        SpecialState* st = State(player);

        int32 wantSpark = PlayerHasSpecialItem(player, ITEM_FEL_SPARK_SIGNET) ? 8 : 0;
        if (wantSpark != st->felSparkSpApplied)
        {
            if (st->felSparkSpApplied)
                player->ApplySpellPowerBonus(st->felSparkSpApplied, false);
            st->felSparkSpApplied = wantSpark;
            if (wantSpark)
                player->ApplySpellPowerBonus(wantSpark, true);
        }

        int32 wantStaff = PlayerHasSpecialItem(player, ITEM_WORGEN_CALLERS_STAFF) ? 25 : 0;
        if (wantStaff != st->staffSpApplied)
        {
            if (st->staffSpApplied)
                player->ApplySpellPowerBonus(st->staffSpApplied, false);
            st->staffSpApplied = wantStaff;
            if (wantStaff)
                player->ApplySpellPowerBonus(wantStaff, true);
        }

        int32 wantDread = PlayerHasSpecialItem(player, ITEM_DREADLORD_CLAW) ? 35 : 0;
        if (wantDread != st->dreadlordSpApplied)
        {
            if (st->dreadlordSpApplied)
                player->ApplySpellPowerBonus(st->dreadlordSpApplied, false);
            st->dreadlordSpApplied = wantDread;
            if (wantDread)
                player->ApplySpellPowerBonus(wantDread, true);
        }

        int32 wantDim = PlayerHasSpecialItem(player, ITEM_DIMENSIUS_DUST) ? 30 : 0;
        if (wantDim != st->dimensiusSpApplied)
        {
            if (st->dimensiusSpApplied)
                player->ApplySpellPowerBonus(st->dimensiusSpApplied, false);
            st->dimensiusSpApplied = wantDim;
            if (wantDim)
                player->ApplySpellPowerBonus(wantDim, true);
        }

        int32 wantPlague = PlayerHasSpecialItem(player, ITEM_FESTERGUT_PLAGUE) ? 40 : 0;
        if (wantPlague != st->festergutSpApplied)
        {
            if (st->festergutSpApplied)
                player->ApplySpellPowerBonus(st->festergutSpApplied, false);
            st->festergutSpApplied = wantPlague;
            if (wantPlague)
                player->ApplySpellPowerBonus(wantPlague, true);
        }

        if (!PlayerHasSpecialItem(player, ITEM_BLOODSEAL_NETHERKURSE))
            ClearBloodsealStacks(player);

        if (!PlayerHasSpecialItem(player, ITEM_SOULFLAME_LANTERN))
        {
            st->soulflameCharges = 0;
            SyncSoulflameSp(player);
        }
        else
            SyncSoulflameSp(player);

        // Restless Void: +15% movement (and mounted via spell_linked_spell on 26023).
        // Applied here so item_template has no paladin "Pursuit of Justice" Equip line.
        bool wantMs = PlayerHasSpecialItem(player, ITEM_SIGNET_RESTLESS_VOID);
        bool hasMs = player->HasAura(SPELL_RESTLESS_VOID_MS);
        if (wantMs && !hasMs)
            player->CastSpell(player, SPELL_RESTLESS_VOID_MS, true);
        else if (!wantMs && hasMs)
            player->RemoveAurasDueToSpell(SPELL_RESTLESS_VOID_MS);
    }

    bool HandleGenericOnUse(Player* player, Item* item)
    {
        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);
        uint32 entry = item->GetEntry();

        if (OnCooldown(player, entry))
        {
            Send(player, "|cffff8000That relic still recovers its power.|r");
            return true;
        }

        switch (entry)
        {
            case ITEM_STARTERS_SOUL_PIN:
                player->ModifyHealth(player->CountPctFromMaxHealth(5));
                Send(player, "|cff9370dbA stray soul mends your flesh.|r");
                StartCooldown(player, entry, 60000);
                return true;
            case ITEM_ABYSSAL_SOUL_TRINKET:
                player->ModifyPower(POWER_MANA, player->GetMaxPower(POWER_MANA) / 6);
                Send(player, "|cff9370dbThe abyss returns borrowed mana.|r");
                StartCooldown(player, entry, 120000);
                return true;
            case ITEM_MAGTHERIDON_CUBE:
            {
                int32 bp = 800;
                player->CastCustomSpell(player, SPELL_SHADOWFLAME, &bp, nullptr, nullptr, true);
                Send(player, "|cff9370dbFel lightning erupts from the cube shard.|r");
                StartCooldown(player, entry, 180000);
                return true;
            }
            default:
                return false;
        }
    }

    bool HandleBloodsealOnUse(Player* player, Item* item)
    {
        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);
        if (OnCooldown(player, ITEM_BLOODSEAL_NETHERKURSE))
        {
            Send(player, "|cffff8000The Bloodseal demands more time.|r");
            return true;
        }

        bool const hasMeta = player->HasSpell(SPELL_METAMORPHOSIS);
        Pet* pet = player->GetPet();
        bool const petReady = pet && pet->IsAlive();
        if (!hasMeta && !petReady)
        {
            if (pet)
                Send(player, "|cffff8000Your demon must live to claim this power.|r");
            else
                Send(player, "|cffff8000No demon answers the Bloodseal — learn Metamorphosis, or summon a servant.|r");
            return true;
        }

        uint32 burn = player->CountPctFromCurHealth(25);
        if (burn >= player->GetHealth())
            burn = player->GetHealth() - 1;
        if (burn)
            Unit::DealDamage(player, player, burn, nullptr, SELF_DAMAGE, SPELL_SCHOOL_MASK_SHADOW, nullptr, false);

        if (hasMeta)
            player->CastSpell(player, SPELL_METAMORPHOSIS, true);
        else
            player->CastSpell(pet, SPELL_BESTIAL_WRATH, true);

        Send(player, "|cffff4500Fel Apotheosis!|r |cff9370dbYour blood buys borrowed power.|r");
        StartCooldown(player, ITEM_BLOODSEAL_NETHERKURSE, CD_GENERIC_MS);
        return true;
    }

    bool HandleVoidheartOnUse(Player* player, Item* item)
    {
        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);
        if (OnCooldown(player, ITEM_VOIDHEART))
        {
            Send(player, "|cffff8000The Voidheart still hungers.|r");
            return true;
        }

        int32 bp = 1500;
        player->CastCustomSpell(player, SPELL_SHADOWFLAME, &bp, nullptr, nullptr, true);
        State(player)->voidPressure = 0;
        Send(player, "|cff9370dbEvent Horizon — the void collapses inward.|r");
        StartCooldown(player, ITEM_VOIDHEART, CD_GENERIC_MS);
        return true;
    }

    bool HandleNecrolyteOnUse(Player* player, Item* item, SpellCastTargets const& targets)
    {
        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);
        if (OnCooldown(player, ITEM_SEAL_FIRST_NECROLYTE))
        {
            Send(player, "|cffff8000The First Necrolyte's seal cools.|r");
            return true;
        }

        Unit* mark = targets.GetUnitTarget();
        if (!mark || !player->IsValidAttackTarget(mark))
        {
            Send(player, "|cffff8000Choose a foe to reap.|r");
            return true;
        }

        State(player)->reapingTarget = mark->GetGUID();
        State(player)->reapingExpireMs = getMSTime() + 15000;
        Send(player, "|cff9370dbSoul Reaping marks your prey.|r");
        StartCooldown(player, ITEM_SEAL_FIRST_NECROLYTE, 90000);
        return true;
    }

    bool HandleGrimoireOnUse(Player* player, Item* item)
    {
        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);
        if (OnCooldown(player, ITEM_GRIMOIRE_EREDAR_TWINS))
        {
            Send(player, "|cffff8000The Grimoire's pages are still smoldering.|r");
            return true;
        }

        for (uint32 i = 0; i < 3; ++i)
        {
            float angle = player->GetOrientation() + float(i) * 2.094f;
            float x = player->GetPositionX() + std::cos(angle) * 2.f;
            float y = player->GetPositionY() + std::sin(angle) * 2.f;
            if (TempSummon* imp = player->SummonCreature(NPC_IMP, x, y, player->GetPositionZ(), player->GetOrientation(),
                TEMPSUMMON_TIMED_DESPAWN, 15000))
            {
                imp->SetOwnerGUID(player->GetGUID());
                imp->SetFaction(player->GetFaction());
                imp->SetLevel(player->GetLevel());
                if (Unit* target = player->GetSelectedUnit())
                    if (imp->IsValidAttackTarget(target) && imp->AI())
                        imp->AI()->AttackStart(target);
            }
        }

        Send(player, "|cff9370dbLegion Unbound — imps answer the twins' writ.|r");
        StartCooldown(player, ITEM_GRIMOIRE_EREDAR_TWINS, CD_GRIMOIRE_MS);
        return true;
    }

    bool HandleSoulflameOnUse(Player* player, Item* item)
    {
        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);
        if (OnCooldown(player, ITEM_SOULFLAME_LANTERN))
        {
            Send(player, "|cffff8000The lantern's flame gutters low.|r");
            return true;
        }

        SpecialState* st = State(player);
        if (!st->soulflameCharges)
        {
            Send(player, "|cffff8000No souls remain in the lantern.|r");
            return true;
        }

        int32 bp = int32(200 + st->soulflameCharges * 15);
        player->CastCustomSpell(player, SPELL_SHADOW_NOVA, &bp, nullptr, nullptr, true);
        player->ModifyHealth(int32(bp / 2));
        if (Pet* pet = player->GetPet())
            pet->ModifyHealth(int32(bp / 2));

        Send(player, Acore::StringFormat("|cffff4500Open the Lantern!|r |cff9370db{} souls released.|r", st->soulflameCharges));
        st->soulflameCharges = 0;
        SyncSoulflameSp(player);
        StartCooldown(player, ITEM_SOULFLAME_LANTERN, CD_GENERIC_MS);
        return true;
    }
}

bool WarlockSpecialItems::IsSpecialItem(uint32 entry)
{
    for (uint32 id : SCRIPTED_ITEMS)
        if (id == entry)
            return true;
    return false;
}

bool WarlockSpecialItems::PlayerHasSpecialItem(Player* player, uint32 entry)
{
    return player && player->HasItemOrGemWithIdEquipped(entry, 1);
}

// -----------------------------------------------------------------------------
// Unified ItemScript for generic signature on-use items
// -----------------------------------------------------------------------------

class item_warlock_special : public ItemScript
{
public:
    item_warlock_special() : ItemScript("item_warlock_special") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!IsEnabled() || !IsWarlock(player) || !item)
            return true;
        HandleGenericOnUse(player, item);
        return true;
    }
};

class item_bloodseal_nethekurse : public ItemScript
{
public:
    item_bloodseal_nethekurse() : ItemScript("item_bloodseal_nethekurse") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!IsEnabled() || !IsWarlock(player))
            return true;
        return HandleBloodsealOnUse(player, item);
    }
};

class item_voidheart : public ItemScript
{
public:
    item_voidheart() : ItemScript("item_voidheart") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!IsEnabled() || !IsWarlock(player))
            return true;
        return HandleVoidheartOnUse(player, item);
    }
};

class item_seal_first_necrolyte : public ItemScript
{
public:
    item_seal_first_necrolyte() : ItemScript("item_seal_first_necrolyte") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        if (!IsEnabled() || !IsWarlock(player))
            return true;
        return HandleNecrolyteOnUse(player, item, targets);
    }
};

class item_grimoire_eredar_twins : public ItemScript
{
public:
    item_grimoire_eredar_twins() : ItemScript("item_grimoire_eredar_twins") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!IsEnabled() || !IsWarlock(player))
            return true;
        return HandleGrimoireOnUse(player, item);
    }
};

class item_soulflame_lantern : public ItemScript
{
public:
    item_soulflame_lantern() : ItemScript("item_soulflame_lantern") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!IsEnabled() || !IsWarlock(player))
            return true;
        return HandleSoulflameOnUse(player, item);
    }
};

// -----------------------------------------------------------------------------
// PlayerScript — equip sync, spell hooks, kill rewards
// -----------------------------------------------------------------------------

class warlock_special_items_playerscript : public PlayerScript
{
public:
    warlock_special_items_playerscript() : PlayerScript("warlock_special_items_playerscript",
        {
            PLAYERHOOK_ON_LOGIN,
            PLAYERHOOK_ON_EQUIP,
            PLAYERHOOK_ON_UNEQUIP_ITEM,
            PLAYERHOOK_ON_SPELL_CAST,
            PLAYERHOOK_ON_CREATURE_KILL,
            PLAYERHOOK_ON_CREATURE_KILLED_BY_PET
        }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (IsWarlock(player))
            SyncEquipPassives(player);
    }

    void OnPlayerEquip(Player* player, Item* item, uint8 /*bag*/, uint8 /*slot*/, bool /*update*/) override
    {
        if (!item || !IsWarlock(player))
            return;
        if (IsSpecialItem(item->GetEntry()))
            SyncEquipPassives(player);
    }

    void OnPlayerUnequip(Player* player, Item* item) override
    {
        if (!item || !IsWarlock(player))
            return;
        if (IsSpecialItem(item->GetEntry()))
            SyncEquipPassives(player);
    }

    void OnPlayerSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (!IsEnabled() || !IsWarlock(player) || !spell)
            return;

        SpellInfo const* info = spell->GetSpellInfo();
        if (!info)
            return;

        // Bloodseal: Life Tap stacks spell power
        if (PlayerHasSpecialItem(player, ITEM_BLOODSEAL_NETHERKURSE) && IsLifeTapSpell(info))
        {
            SpecialState* st = State(player);
            if (st->bloodsealStacks < 10)
            {
                ++st->bloodsealStacks;
                st->bloodsealSpApplied += 15;
                player->ApplySpellPowerBonus(15, true);
            }
        }

        // Grimoire: echo direct damage to pet
        if (PlayerHasSpecialItem(player, ITEM_GRIMOIRE_EREDAR_TWINS)
            && info->DmgClass == SPELL_DAMAGE_CLASS_MAGIC
            && info->HasEffect(SPELL_EFFECT_SCHOOL_DAMAGE))
        {
            if (Pet* pet = player->GetPet())
            {
                if (Unit* target = spell->m_targets.GetUnitTarget())
                    if (pet->IsWithinDist(target, 30.f))
                        pet->CastSpell(target, SPELL_IMMOLATE, true);
            }
        }

        // Voidheart: Corruption becomes permanent. Pinned after the cast lands,
        // because at hook time the new aura application has not happened yet.
        if (PlayerHasSpecialItem(player, ITEM_VOIDHEART) && IsCorruptionSpell(info))
        {
            uint32 corruptionId = info->Id;
            if (Unit* target = spell->m_targets.GetUnitTarget())
            {
                ObjectGuid targetGuid = target->GetGUID();
                player->m_Events.AddEventAtOffset([player, targetGuid, corruptionId]()
                {
                    if (!PlayerHasSpecialItem(player, ITEM_VOIDHEART))
                        return;
                    if (Unit* victim = ObjectAccessor::GetUnit(*player, targetGuid))
                        if (Aura* aura = victim->GetAura(corruptionId, player->GetGUID()))
                        {
                            aura->SetMaxDuration(-1);
                            aura->SetDuration(-1);
                        }
                }, Milliseconds(500));
            }
        }
    }

    void HandleKill(Player* player, Unit* victim)
    {
        if (!IsEnabled() || !IsWarlock(player) || !victim || !victim->IsCreature())
            return;

        if (PlayerHasSpecialItem(player, ITEM_IMMOLTHAR_MANAFEED))
            player->ModifyPower(POWER_MANA, 50);

        if (PlayerHasSpecialItem(player, ITEM_DEATHS_HEAD_SOUL_PIN))
        {
            SpecialState* st = State(player);
            if (st->deathsHeadSpApplied + DEATHS_HEAD_SP_PER_KILL <= DEATHS_HEAD_SP_CAP)
            {
                st->deathsHeadSpApplied += DEATHS_HEAD_SP_PER_KILL;
                player->ApplySpellPowerBonus(DEATHS_HEAD_SP_PER_KILL, true);
                player->m_Events.AddEventAtOffset([player]()
                {
                    SpecialState* state = State(player);
                    if (state->deathsHeadSpApplied >= DEATHS_HEAD_SP_PER_KILL)
                    {
                        state->deathsHeadSpApplied -= DEATHS_HEAD_SP_PER_KILL;
                        player->ApplySpellPowerBonus(DEATHS_HEAD_SP_PER_KILL, false);
                    }
                }, Milliseconds(15000));
            }
        }

        if (PlayerHasSpecialItem(player, ITEM_PRINCESS_SOUL_LOCKET) && victim->GetCreatureType() == CREATURE_TYPE_ELEMENTAL)
            player->ModifyPower(POWER_MANA, 150);

        if (PlayerHasSpecialItem(player, ITEM_SOULFLAME_LANTERN))
        {
            SpecialState* st = State(player);
            if (st->soulflameCharges < 100)
            {
                ++st->soulflameCharges;
                SyncSoulflameSp(player);
            }
        }

        // Soul Reaping payout
        SpecialState* st = State(player);
        if (st->reapingExpireMs > getMSTime() && victim->GetGUID() == st->reapingTarget)
        {
            player->ModifyPower(POWER_MANA, player->GetMaxPower(POWER_MANA) / 4);
            player->ModifyHealth(player->CountPctFromMaxHealth(15));
            Send(player, "|cff9370dbSoul Reaping — the mark pays its due.|r");
            st->reapingTarget.Clear();
            st->reapingExpireMs = 0;
        }
    }

    void OnPlayerCreatureKill(Player* killer, Creature* killed) override
    {
        HandleKill(killer, killed);
    }

    void OnPlayerCreatureKilledByPet(Player* owner, Creature* killed) override
    {
        HandleKill(owner, killed);
    }
};

// -----------------------------------------------------------------------------
// UnitScript — damage, healing, corruption spread
// -----------------------------------------------------------------------------

class warlock_special_items_unitscript : public UnitScript
{
public:
    warlock_special_items_unitscript() : UnitScript("warlock_special_items_unitscript", true,
        {
            UNITHOOK_MODIFY_SPELL_DAMAGE_TAKEN,
            UNITHOOK_MODIFY_PERIODIC_DAMAGE_AURAS_TICK,
            UNITHOOK_MODIFY_HEAL_RECEIVED,
            UNITHOOK_ON_UNIT_DEATH
        }) { }

    // Shared by direct spell damage and DoT ticks. The Void Pressure nova is
    // deferred to the player's event queue: casting a new spell while the
    // engine is still computing damage for the current one invites reentrancy.
    template <typename DamageT>
    void HandleDamageDone(Unit* attacker, DamageT& damage, SpellInfo const* spellInfo)
    {
        if (!IsEnabled() || !attacker || !spellInfo || damage <= DamageT(0))
            return;

        Player* player = attacker->ToPlayer();
        if (!player || !IsWarlock(player))
            return;

        if (PlayerHasSpecialItem(player, ITEM_VOIDHEART)
            && (spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_SHADOW))
        {
            damage += CalculatePct(damage, 15);
            SpecialState* st = State(player);
            st->voidPressure += uint32(damage);
            if (st->voidPressure >= 8000 && !st->voidNovaPending)
            {
                st->voidNovaPending = true;
                player->m_Events.AddEventAtOffset([player]()
                {
                    SpecialState* state = State(player);
                    state->voidNovaPending = false;
                    state->voidPressure = 0;
                    if (!PlayerHasSpecialItem(player, ITEM_VOIDHEART))
                        return;
                    int32 bp = 600;
                    player->CastCustomSpell(player, SPELL_SHADOW_NOVA, &bp, nullptr, nullptr, true);
                }, Milliseconds(100));
            }
        }

        if (PlayerHasSpecialItem(player, ITEM_DIMENSIUS_DUST)
            && (spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_SHADOW))
            damage += CalculatePct(damage, 10);

        if (PlayerHasSpecialItem(player, ITEM_ARAN_EMBER_CLOAK)
            && (spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_FIRE))
            damage += CalculatePct(damage, 8);
    }

    void ModifySpellDamageTaken(Unit* /*target*/, Unit* attacker, int32& damage, SpellInfo const* spellInfo) override
    {
        HandleDamageDone(attacker, damage, spellInfo);
    }

    void ModifyPeriodicDamageAurasTick(Unit* /*target*/, Unit* attacker, uint32& damage, SpellInfo const* spellInfo) override
    {
        HandleDamageDone(attacker, damage, spellInfo);
    }

    void ModifyHealReceived(Unit* target, Unit* healer, uint32& heal, SpellInfo const* spellInfo) override
    {
        if (!IsEnabled() || !target || !heal)
            return;

        Player* player = target->ToPlayer();
        if (!player || !IsWarlock(player))
            return;

        // Bloodseal: outside healing withers. The warlock's own drains and
        // self-heals are exempt — life bought with blood stays bought.
        if (PlayerHasSpecialItem(player, ITEM_BLOODSEAL_NETHERKURSE) && healer && healer != target)
            heal -= CalculatePct(heal, 30);

        if (PlayerHasSpecialItem(player, ITEM_VOIDHEART) && spellInfo
            && (spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_HOLY))
            heal -= CalculatePct(heal, 40);

        // Seal of the First Necrolyte: mastery of the original death magic —
        // Drain Life restores 50% more.
        if (PlayerHasSpecialItem(player, ITEM_SEAL_FIRST_NECROLYTE) && IsDrainLifeSpell(spellInfo))
            heal += CalculatePct(heal, 50);
    }

    void OnUnitDeath(Unit* unit, Unit* killer) override
    {
        if (!IsEnabled() || !unit || !killer)
            return;

        Player* player = killer->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!player || !PlayerHasSpecialItem(player, ITEM_VOIDHEART))
            return;

        if (!FindPlayerCorruption(unit, player->GetGUID()))
            return;

        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(unit, player, 10.f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(unit, targets, check);
        Cell::VisitObjects(unit, searcher, 10.f);

        for (Unit* tgt : targets)
        {
            if (tgt == unit || !player->IsValidAttackTarget(tgt))
                continue;
            if (!FindPlayerCorruption(tgt, player->GetGUID()))
                player->AddAura(SPELL_CORRUPTION_MAX, tgt);
            break;
        }
    }
};

void AddSC_warlock_special_items()
{
    new item_warlock_special();
    new item_bloodseal_nethekurse();
    new item_voidheart();
    new item_seal_first_necrolyte();
    new item_grimoire_eredar_twins();
    new item_soulflame_lantern();
    new warlock_special_items_playerscript();
    new warlock_special_items_unitscript();
}
