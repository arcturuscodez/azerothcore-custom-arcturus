/*
 * Crimson Shade — rogue Stealth / Meta-style stance.
 *
 * 90030: FORM_STEALTH shapeshift + MOD_STEALTH + speed (no OVERRIDE_SPELLS).
 * Openers: permanently taught at Dread Warlock; Stances = FORM_STEALTH (like Ambush).
 * Pet is not forced into stealth; owner Shade alone lets the demon engage while the
 * warlock stays out of combat until they act or take damage.
 */

#include "warlock_arcturus_spells.h"

#include "SpellScript.h"
#include "SpellScriptLoader.h"
#include "Unit.h"

using namespace ArcturusSpells;

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

void AddSC_warlock_crimson_shade()
{
    RegisterSpellScript(spell_crimson_opener_strip_shade);
    RegisterSpellScript(spell_withering_touch);
    RegisterSpellScript(spell_tormenting_rend);
}
