/*
 * Wrath of Chaos (90046) — Soul Reaver (5000 souls) DoT applicator.
 *
 * A convenience wrapper, not a new DoT: on hit it casts the caster's own highest known
 * Corruption, Curse of Agony, and Immolate — Unstable Affliction taking the Immolate slot
 * when the caster has it. Casting the stock spells is the whole point: talents, glyphs,
 * spell power coefficients, the Agony ramp, Nightfall / Molten Core procs, and
 * Conflagrate-on-Immolate all keep working because the auras are the real ones.
 *
 * Rank walk: step GetNextRankSpell() from rank 1 and keep the last id the caster actually
 * knows. Do not stop at the first gap — WotLK unlearns superseded ranks, so a leftover
 * rank 1 with a hole at rank 2 is common. Breaking there would apply rank 1 forever.
 */

#include "warlock_arcturus_spells.h"

#include "Player.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "SpellScriptLoader.h"
#include "Unit.h"

using namespace ArcturusSpells;

namespace
{
    constexpr uint32 SPELL_WARLOCK_CORRUPTION_R1          = 172;
    constexpr uint32 SPELL_WARLOCK_CURSE_OF_AGONY_R1      = 980;
    constexpr uint32 SPELL_WARLOCK_IMMOLATE_R1            = 348;
    constexpr uint32 SPELL_WARLOCK_UNSTABLE_AFFLICTION_R1 = 30108;

    // Highest rank of a chain the caster has learned; 0 when the family is unknown.
    uint32 HighestKnownRank(Unit const* caster, uint32 firstRankId)
    {
        Player const* player = caster->ToPlayer();
        if (!player)
            return 0;

        uint32 known = 0;
        for (SpellInfo const* info = sSpellMgr->GetSpellInfo(firstRankId); info; info = info->GetNextRankSpell())
            if (player->HasSpell(info->Id))
                known = info->Id;

        return known;
    }

    // Unstable Affliction and Immolate share one slot on the wrapper: UA wins when known,
    // and stock Immolate stays on the bar for Conflagrate / Molten Core.
    uint32 ImmolateOrUnstableAffliction(Unit const* caster)
    {
        if (uint32 unstableAffliction = HighestKnownRank(caster, SPELL_WARLOCK_UNSTABLE_AFFLICTION_R1))
            return unstableAffliction;

        return HighestKnownRank(caster, SPELL_WARLOCK_IMMOLATE_R1);
    }
}

class spell_warlock_wrath_of_chaos : public SpellScript
{
    PrepareSpellScript(spell_warlock_wrath_of_chaos);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({
            SPELL_WARLOCK_CORRUPTION_R1,
            SPELL_WARLOCK_CURSE_OF_AGONY_R1,
            SPELL_WARLOCK_IMMOLATE_R1,
            SPELL_WARLOCK_UNSTABLE_AFFLICTION_R1
        });
    }

    SpellCastResult CheckCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return SPELL_FAILED_ERROR;

        // Crimson Shade already remaps these three into its own openers. ExcludeCasterAuraSpell
        // greys the button client-side; this keeps the refusal explicit server-side too.
        if (caster->HasAura(SPELL_CRIMSON_SHADE))
            return SPELL_FAILED_CASTER_AURASTATE;

        // Nothing learned yet — do not burn the cast time and the mana on a no-op.
        if (!HighestKnownRank(caster, SPELL_WARLOCK_CORRUPTION_R1)
            && !HighestKnownRank(caster, SPELL_WARLOCK_CURSE_OF_AGONY_R1)
            && !ImmolateOrUnstableAffliction(caster))
            return SPELL_FAILED_NOT_KNOWN;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        // Triggered: the wrapper already paid mana and the GCD, so each DoT lands free.
        ApplyIfKnown(caster, target, HighestKnownRank(caster, SPELL_WARLOCK_CORRUPTION_R1));
        ApplyIfKnown(caster, target, HighestKnownRank(caster, SPELL_WARLOCK_CURSE_OF_AGONY_R1));
        ApplyIfKnown(caster, target, ImmolateOrUnstableAffliction(caster));
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_warlock_wrath_of_chaos::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_warlock_wrath_of_chaos::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }

private:
    static void ApplyIfKnown(Unit* caster, Unit* target, uint32 spellId)
    {
        if (spellId)
            caster->CastSpell(target, spellId, true);
    }
};

void AddSC_warlock_wrath_of_chaos()
{
    RegisterSpellScript(spell_warlock_wrath_of_chaos);
}
