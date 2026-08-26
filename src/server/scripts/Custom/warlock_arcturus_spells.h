/*
 * Arcturus warlock custom spell IDs (Felguard Mandate, Crimson Shade, Malkoron).
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
    constexpr uint32 SPELL_FELGUARD_MANDATE         = 90024;
    constexpr uint32 SPELL_LEGION_BRAND_AMP         = 90026; // owner-cast +% taken from warlock

    // Crimson Shade
    constexpr uint32 SPELL_CRIMSON_SHADE            = 90030;
    constexpr uint32 SPELL_SOUL_REAVING             = 90031;
    constexpr uint32 SPELL_SEARING_BRAND            = 90032;
    constexpr uint32 SPELL_TORMENTING_REND          = 90033;
    constexpr uint32 SPELL_WITHERING_TOUCH          = 90034;
    constexpr uint32 SPELL_WITHERING_AMP            = 90035;
    constexpr uint32 SPELL_TORMENTING_SILENCE       = 90036;

    constexpr uint32 BRAND_OWNER_DAMAGE_PCT         = 8;
    constexpr uint32 FELSTORM_BRAND_BONUS_PCT       = 25;
    constexpr uint32 BRAND_SOUL_CAP                 = 500;

    // Malkoron, Blade of the Soul-Eater (item 900100) — item-bound, not Chaos-taught
    constexpr uint32 SPELL_SOULPIKE                 = 90100;
    constexpr uint32 SPELL_TORMENT_FRAGMENT         = 90101;
    constexpr uint32 SPELL_SOUL_BANE                = 90102;
    constexpr uint32 SPELL_FEL_ATTUNEMENT           = 90103;
    constexpr uint32 SPELL_DAMNED_CONCORD           = 90104;
    constexpr uint32 SPELL_CHAOTIC_VISION           = 90105;
    constexpr uint32 SPELL_CALL_OF_THE_REAVER       = 90106;
    constexpr uint32 SPELL_DAMNED_CONCORD_PET       = 90107;
    constexpr uint32 ITEM_MALKORON                  = 900100;
}

#endif
