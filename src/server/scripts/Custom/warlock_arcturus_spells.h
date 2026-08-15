/*
 * Arcturus warlock custom spell IDs (Felguard Mandate + Crimson Shade).
 * Shared by Custom scripts; PetDefines keeps pet learn aliases separately.
 */

#ifndef _WARLOCK_ARCTURUS_SPELLS_H_
#define _WARLOCK_ARCTURUS_SPELLS_H_

#include "Define.h"

namespace ArcturusSpells
{
    // Existing Felstorm
    constexpr uint32 SPELL_FELSTORM                 = 90019;
    constexpr uint32 SPELL_FELSTORM_TICK            = 90020;

    // Felguard Legion Mandate
    constexpr uint32 SPELL_LEGION_BRAND_PASSIVE     = 90021;
    constexpr uint32 SPELL_LEGION_BRAND             = 90022;
    constexpr uint32 SPELL_FELSTORM_THREAT_PULSE    = 90023;
    constexpr uint32 SPELL_FELGUARD_MANDATE         = 90024;
    // 90025 Pursuit Mark — retired (deleted from spell_dbc).
    constexpr uint32 SPELL_LEGION_BRAND_AMP         = 90026; // owner-cast +% taken from warlock

    // Crimson Shade
    constexpr uint32 SPELL_CRIMSON_SHADE            = 90030;
    constexpr uint32 SPELL_SOUL_REAVING             = 90031;
    constexpr uint32 SPELL_SEARING_BRAND            = 90032;
    constexpr uint32 SPELL_TORMENTING_REND          = 90033;
    constexpr uint32 SPELL_WITHERING_TOUCH          = 90034;
    constexpr uint32 SPELL_WITHERING_AMP            = 90035;
    constexpr uint32 SPELL_TORMENTING_SILENCE       = 90036;
    // 90037 pet stealth — retired (deleted from spell_dbc); Shade does not force pet stealth.

    // FORM_STEALTH stance mask (1 << (FORM_STEALTH - 1)) — Ambush / Meta-style gate
    constexpr uint32 STANCE_MASK_STEALTH            = 536870912u;

    constexpr uint32 BRAND_OWNER_DAMAGE_PCT         = 8;
    constexpr uint32 FELSTORM_BRAND_BONUS_PCT       = 25;
    constexpr uint32 BRAND_SOUL_CAP                 = 500;
}

#endif
