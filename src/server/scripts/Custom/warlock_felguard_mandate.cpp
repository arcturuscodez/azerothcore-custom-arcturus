/*
 * Felguard Legion Mandate — Brand, Felstorm upgrades, Intercept brand, DE Mandate.
 */

#include "warlock_arcturus_spells.h"
#include "warlock_demonic_empowerment.h"

#include "Pet.h"
#include "PetDefines.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include "SpellScriptLoader.h"
#include "Unit.h"
#include "WorldPacket.h"

using namespace ArcturusSpells;

namespace
{
    Player* OwnerPlayerOf(Unit* unit)
    {
        if (!unit)
            return nullptr;
        if (Unit* owner = unit->GetOwner())
            return owner->ToPlayer();
        return nullptr;
    }

    // DoT from the Felguard (threat stays on the tank). Owner-only amp is 90026.
    void ApplyLegionBrand(Unit* pet, Unit* target)
    {
        if (!pet || !target || !target->IsAlive())
            return;

        pet->CastSpell(target, SPELL_LEGION_BRAND, true);
        if (Player* owner = OwnerPlayerOf(pet))
            owner->CastSpell(target, SPELL_LEGION_BRAND_AMP, true);
    }

    uint32 BrandSoulPoints(Unit* pet)
    {
        Player* owner = OwnerPlayerOf(pet);
        if (!owner)
            return 0;
        if (!sWarlockEmpower->IsLoaded(owner->GetGUID()))
            sWarlockEmpower->LoadFromDB(owner->GetGUID());
        WarlockEmpowerment::Souls souls = sWarlockEmpower->Get(owner->GetGUID());
        uint32 applied = WarlockEmpowerment::AppliedSoulsFor(souls.current);
        if (applied > BRAND_SOUL_CAP)
            applied = BRAND_SOUL_CAP;
        return applied;
    }

    void SyncPetSpellCooldown(Pet* pet, uint32 spellId, uint32 remainingMs)
    {
        if (!pet)
            return;
        Player* owner = pet->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!owner)
            return;

        WorldPacket data;
        pet->BuildCooldownPacket(data, 0, spellId, remainingMs);
        owner->SendDirectMessage(&data);
    }
}

class spell_felguard_felstorm_tick : public SpellScript
{
    PrepareSpellScript(spell_felguard_felstorm_tick);

    void RecalculateDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        // Brand first so this tick can receive the +25% amp.
        ApplyLegionBrand(caster, target);

        float const ap = caster->GetTotalAttackPowerValue(BASE_ATTACK);
        int32 damage = int32(caster->GetLevel() * 4) + int32(ap * 0.18f);
        if (target->HasAura(SPELL_LEGION_BRAND))
            damage += CalculatePct(damage, FELSTORM_BRAND_BONUS_PCT);
        SetHitDamage(damage > 0 ? damage : 1);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_felguard_felstorm_tick::RecalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_felguard_cleave_brand : public SpellScript
{
    PrepareSpellScript(spell_felguard_cleave_brand);

    void HandleHit()
    {
        Unit* caster = GetCaster();
        if (!caster || caster->GetEntry() != NPC_FELGUARD)
            return;
        ApplyLegionBrand(caster, GetHitUnit());
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_felguard_cleave_brand::HandleHit);
    }
};

class spell_felguard_intercept_pursuit : public SpellScript
{
    PrepareSpellScript(spell_felguard_intercept_pursuit);

    void HandleChargeHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || caster->GetEntry() != NPC_FELGUARD || !target)
            return;

        ApplyLegionBrand(caster, target);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_felguard_intercept_pursuit::HandleChargeHit, EFFECT_0, SPELL_EFFECT_CHARGE);
    }
};

class spell_legion_brand_aura : public AuraScript
{
    PrepareAuraScript(spell_legion_brand_aura);

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;
        float const ap = caster->GetTotalAttackPowerValue(BASE_ATTACK);
        uint32 const souls = BrandSoulPoints(caster);
        amount = int32(caster->GetLevel() * 2) + int32(ap * 0.05f) + int32(souls / 10);
        if (amount < 1)
            amount = 1;
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_legion_brand_aura::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

class spell_legion_brand_amp_aura : public AuraScript
{
    PrepareAuraScript(spell_legion_brand_amp_aura);

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount = int32(BRAND_OWNER_DAMAGE_PCT);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_legion_brand_amp_aura::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_DAMAGE_FROM_CASTER);
    }
};

class spell_felguard_mandate_aura : public AuraScript
{
    PrepareAuraScript(spell_felguard_mandate_aura);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        Pet* pet = target ? target->ToPet() : nullptr;
        if (!pet || pet->GetEntry() != NPC_FELGUARD)
            return;
        pet->RemoveSpellCooldown(SPELL_FELSTORM, true);
        SyncPetSpellCooldown(pet, SPELL_FELSTORM, 0);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_felguard_mandate_aura::OnApply, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_warlock_felguard_mandate()
{
    RegisterSpellScript(spell_felguard_felstorm_tick);
    RegisterSpellScript(spell_felguard_cleave_brand);
    RegisterSpellScript(spell_felguard_intercept_pursuit);
    RegisterSpellScript(spell_legion_brand_aura);
    RegisterSpellScript(spell_legion_brand_amp_aura);
    RegisterSpellScript(spell_felguard_mandate_aura);
}
