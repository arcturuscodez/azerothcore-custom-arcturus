/*
 * Warlock Legendaries — Cinderfury + Noggenfogger only.
 *
 * ScriptMgr registrations and behaviour in warlock_legendaries.cpp.
 */

#ifndef _WARLOCK_LEGENDARIES_H_
#define _WARLOCK_LEGENDARIES_H_

#include "Define.h"

namespace WarlockLegendaries
{
    constexpr uint32 ITEM_NOGGENFOGGER_MAGNUM_OPUS = 900016; // toggle stock Noggenfogger Elixir morph (16591)
    constexpr uint32 ITEM_CINDERFURY              = 900017; // Molten Core exclusive fire kit

    constexpr char const* CONFIG_ENABLED = "WarlockLegendary.Enable";
}

#endif // _WARLOCK_LEGENDARIES_H_
