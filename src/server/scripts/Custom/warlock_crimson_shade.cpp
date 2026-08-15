/*
 * Crimson Shade — ethereal stealth stance + stock-spell opener redirects.
 */

#include "warlock_arcturus_spells.h"

#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellScript.h"
#include "SpellScriptLoader.h"
#include "Unit.h"

using namespace ArcturusSpells;

namespace
{
    // Do not strip Shade here — opener BeforeCast does that after the opener
    // passes range/shape checks. Failed opener casts leave Shade intact.
    SpellCastResult TryShadeRedirect(SpellScript* self, uint32 openerId)
    {
        Unit* caster = self->GetCaster();
        if (!caster || !caster->IsPlayer() || !caster->HasAura(SPELL_CRIMSON_SHADE))
            return SPELL_CAST_OK;

        Unit* target = self->GetExplTargetUnit();
        if (!target)
            return SPELL_FAILED_BAD_TARGETS;

        // Melee openers: keep Shade if out of range (do not strip-then-fail).
        // Opener keeps GCD (no IGNORE_GCD). Power ignored so SB+opener don't double-tax.
        TriggerCastFlags const flags = TriggerCastFlags(
            TRIGGERED_IGNORE_POWER_AND_REAGENT_COST |
            TRIGGERED_IGNORE_CAST_IN_PROGRESS |
            TRIGGERED_IGNORE_SHAPESHIFT);

        SpellCastResult const res = caster->CastSpell(target, openerId, flags);
        if (res != SPELL_CAST_OK)
            return res;

        return SPELL_FAILED_DONT_REPORT;
    }
}

class spell_crimson_redirect_shadow_bolt : public SpellScript
{
    PrepareSpellScript(spell_crimson_redirect_shadow_bolt);

    SpellCastResult CheckCast() { return TryShadeRedirect(this, SPELL_SOUL_REAVING); }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_crimson_redirect_shadow_bolt::CheckCast);
    }
};

class spell_crimson_redirect_immolate : public SpellScript
{
    PrepareSpellScript(spell_crimson_redirect_immolate);

    SpellCastResult CheckCast() { return TryShadeRedirect(this, SPELL_SEARING_BRAND); }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_crimson_redirect_immolate::CheckCast);
    }
};

class spell_crimson_redirect_coa : public SpellScript
{
    PrepareSpellScript(spell_crimson_redirect_coa);

    SpellCastResult CheckCast() { return TryShadeRedirect(this, SPELL_TORMENTING_REND); }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_crimson_redirect_coa::CheckCast);
    }
};

class spell_crimson_redirect_corruption : public SpellScript
{
    PrepareSpellScript(spell_crimson_redirect_corruption);

    SpellCastResult CheckCast() { return TryShadeRedirect(this, SPELL_WITHERING_TOUCH); }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_crimson_redirect_corruption::CheckCast);
    }
};

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
    RegisterSpellScript(spell_crimson_redirect_shadow_bolt);
    RegisterSpellScript(spell_crimson_redirect_immolate);
    RegisterSpellScript(spell_crimson_redirect_coa);
    RegisterSpellScript(spell_crimson_redirect_corruption);
    RegisterSpellScript(spell_crimson_opener_strip_shade);
    RegisterSpellScript(spell_withering_touch);
    RegisterSpellScript(spell_tormenting_rend);
}
