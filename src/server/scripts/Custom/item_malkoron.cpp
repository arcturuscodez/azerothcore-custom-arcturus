/*
 * Malkoron, Blade of the Soul-Eater (item 900100) — warlock legendary polearm.
 *
 *   90100  Soulpike          hidden On Equip engine; procs Torment Fragments
 *   90101  Torment Fragment  stacking buff (cap 10)
 *   90102  Soul Bane         15 yd shadow nova + 10s magic-damage buff
 *   90104  Damned Concord    hidden dummy; script-buffs the current demon
 *   90107  Damned Concord    hidden pet damage aura
 *
 * Call of the Reaver (90106) is a self-centered Death and Decay (no script).
 */

#include "warlock_arcturus_spells.h"
#include "Config.h"
#include "Pet.h"
#include "Player.h"
#include "Random.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "SpellScriptLoader.h"
#include "Timer.h"
#include "Unit.h"

using namespace ArcturusSpells;

namespace
{
    constexpr char const* CONFIG_PROC_CHANCE = "Arcturus.Malkoron.ProcChance";
    constexpr char const* CONFIG_FRAGMENT_CAP = "Arcturus.Malkoron.FragmentCap";

    constexpr uint32 PROC_CHANCE_DEFAULT = 100;
    constexpr uint8  FRAGMENT_CAP_DEFAULT = 10;
    constexpr uint32 CONFIG_CACHE_MS = 5000u;

    struct MalkoronConfig
    {
        uint32 procChance = PROC_CHANCE_DEFAULT;
        uint8 fragmentCap = FRAGMENT_CAP_DEFAULT;
    };

    MalkoronConfig _cache{};
    uint32 _cacheMs = 0;

    MalkoronConfig const& LoadedConfig()
    {
        uint32 const now = getMSTime();
        if (!_cacheMs || getMSTimeDiff(_cacheMs, now) >= CONFIG_CACHE_MS)
        {
            uint32 chance = sConfigMgr->GetOption<uint32>(CONFIG_PROC_CHANCE, PROC_CHANCE_DEFAULT);
            if (chance > 100)
                chance = 100;
            _cache.procChance = chance;

            uint32 cap = sConfigMgr->GetOption<uint32>(CONFIG_FRAGMENT_CAP, FRAGMENT_CAP_DEFAULT);
            if (!cap)
                cap = FRAGMENT_CAP_DEFAULT;
            if (cap > 10)
                cap = 10;
            _cache.fragmentCap = uint8(cap);
            _cacheMs = now ? now : 1u;
        }
        return _cache;
    }

    void EnsurePetConcord(Unit* owner)
    {
        Player* player = owner->ToPlayer();
        if (!player)
            return;

        Pet* pet = player->GetPet();
        if (!pet || !pet->IsAlive())
            return;

        if (!pet->HasAura(SPELL_DAMNED_CONCORD_PET))
            pet->CastSpell(pet, SPELL_DAMNED_CONCORD_PET, true);
    }
}

class spell_item_malkoron_soulpike : public AuraScript
{
    PrepareAuraScript(spell_item_malkoron_soulpike);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_TORMENT_FRAGMENT, SPELL_SOUL_BANE, SPELL_CALL_OF_THE_REAVER });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* owner = GetTarget();
        if (owner->HasAura(SPELL_SOUL_BANE))
            return false;

        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!procSpell)
            return false;

        if (procSpell->Id == SPELL_SOUL_BANE || procSpell->Id == SPELL_CALL_OF_THE_REAVER
            || procSpell->Id == SPELL_TORMENT_FRAGMENT || procSpell->Id == SPELL_SOULPIKE)
            return false;

        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return false;

        return roll_chance_i(int32(LoadedConfig().procChance));
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();
        Unit* owner = GetTarget();
        owner->CastSpell(owner, SPELL_TORMENT_FRAGMENT, true, nullptr, aurEff);

        Aura* fragments = owner->GetAura(SPELL_TORMENT_FRAGMENT);
        if (!fragments)
            return;

        if (fragments->GetStackAmount() >= LoadedConfig().fragmentCap)
        {
            owner->CastSpell(owner, SPELL_SOUL_BANE, true, nullptr, aurEff);
            fragments->Remove();
        }
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->RemoveAurasDueToSpell(SPELL_TORMENT_FRAGMENT);
        GetTarget()->RemoveAurasDueToSpell(SPELL_SOUL_BANE);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_item_malkoron_soulpike::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_item_malkoron_soulpike::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        AfterEffectRemove += AuraEffectRemoveFn(spell_item_malkoron_soulpike::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_item_malkoron_damned_concord : public AuraScript
{
    PrepareAuraScript(spell_item_malkoron_damned_concord);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DAMNED_CONCORD_PET });
    }

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        EnsurePetConcord(GetTarget());
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;

        if (Pet* pet = player->GetPet())
            pet->RemoveAurasDueToSpell(SPELL_DAMNED_CONCORD_PET);
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        EnsurePetConcord(GetTarget());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_item_malkoron_damned_concord::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_item_malkoron_damned_concord::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_item_malkoron_damned_concord::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

void AddSC_item_malkoron()
{
    RegisterSpellScript(spell_item_malkoron_soulpike);
    RegisterSpellScript(spell_item_malkoron_damned_concord);
}
