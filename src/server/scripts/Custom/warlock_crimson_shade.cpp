/*
 * Crimson Shade — rogue Stealth / Meta-style stance.
 *
 * 90030: FORM_STEALTH shapeshift + MOD_STEALTH + speed (no OVERRIDE_SPELLS).
 * Openers: permanently taught at Dread Warlock; Stances = FORM_STEALTH (like Ambush).
 * Pet: 90037 MOD_STEALTH while owner is in Shade.
 */

#include "warlock_arcturus_spells.h"

#include "Pet.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include "SpellScriptLoader.h"
#include "Unit.h"

using namespace ArcturusSpells;

namespace
{
    void SyncShadePetStealth(Unit* owner, bool apply)
    {
        Player* player = owner ? owner->ToPlayer() : nullptr;
        if (!player)
            return;

        Pet* pet = player->GetPet();
        if (!pet || !pet->IsAlive())
            return;

        if (apply)
        {
            if (!pet->HasAura(SPELL_CRIMSON_SHADE_PET))
                pet->CastSpell(pet, SPELL_CRIMSON_SHADE_PET, true);
        }
        else
            pet->RemoveAurasDueToSpell(SPELL_CRIMSON_SHADE_PET);
    }
}

class spell_crimson_shade_aura : public AuraScript
{
    PrepareAuraScript(spell_crimson_shade_aura);

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        SyncShadePetStealth(GetTarget(), true);
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        SyncShadePetStealth(GetTarget(), false);
    }

    void Register() override
    {
        // Effect 0 = MOD_SHAPESHIFT (FORM_STEALTH) after Stealth-style rewrite.
        AfterEffectApply += AuraEffectApplyFn(spell_crimson_shade_aura::HandleApply, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_crimson_shade_aura::HandleRemove, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
    }
};

// Ambush-style: using an opener ends Shade (breaks stealth).
class spell_crimson_opener_strip_shade : public SpellScript
{
    PrepareSpellScript(spell_crimson_opener_strip_shade);

    void HandleBeforeCast()
    {
        if (Unit* caster = GetCaster())
            caster->RemoveAurasDueToSpell(SPELL_CRIMSON_SHADE);
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_crimson_opener_strip_shade::HandleBeforeCast);
    }
};

class spell_withering_touch : public SpellScript
{
    PrepareSpellScript(spell_withering_touch);

    void HandleHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;
        caster->CastSpell(target, SPELL_WITHERING_AMP, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_withering_touch::HandleHit);
    }
};

class spell_tormenting_rend : public SpellScript
{
    PrepareSpellScript(spell_tormenting_rend);

    void HandleHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;
        caster->CastSpell(target, SPELL_TORMENTING_SILENCE, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_tormenting_rend::HandleHit);
    }
};

class crimson_shade_player_script : public PlayerScript
{
public:
    crimson_shade_player_script() : PlayerScript("crimson_shade_player_script", {
        PLAYERHOOK_ON_AFTER_GUARDIAN_INIT_STATS_FOR_LEVEL
    }) { }

    void OnPlayerAfterGuardianInitStatsForLevel(Player* player, Guardian* guardian) override
    {
        if (!player || !guardian || !player->HasAura(SPELL_CRIMSON_SHADE))
            return;
        if (!guardian->ToPet())
            return;
        if (!guardian->HasAura(SPELL_CRIMSON_SHADE_PET))
            guardian->CastSpell(guardian, SPELL_CRIMSON_SHADE_PET, true);
    }
};

void AddSC_warlock_crimson_shade()
{
    RegisterSpellScript(spell_crimson_shade_aura);
    RegisterSpellScript(spell_crimson_opener_strip_shade);
    RegisterSpellScript(spell_withering_touch);
    RegisterSpellScript(spell_tormenting_rend);
    new crimson_shade_player_script();
}
