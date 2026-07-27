/*
 * Warlock Legendaries — Cinderfury + Noggenfogger only.
 *
 * All other 900xxx customs were retired; rebuild one item at a time.
 * ScriptMgr registrations and behaviour in warlock_legendaries.cpp.
 */

#ifndef _WARLOCK_LEGENDARIES_H_
#define _WARLOCK_LEGENDARIES_H_

#include "Define.h"

namespace WarlockLegendaries
{
    constexpr uint32 ITEM_NOGGENFOGGER_MAGNUM_OPUS = 900016; // toggle scaled display morph
    constexpr uint32 ITEM_CINDERFURY              = 900017; // Molten Core exclusive fire kit

    constexpr char const* CONFIG_ENABLED = "WarlockLegendary.Enable";
    constexpr char const* CONFIG_NOGGENFOGGER_SCALE = "WarlockLegendary.Noggenfogger.Scale";
}

#endif // _WARLOCK_LEGENDARIES_H_
