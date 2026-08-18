/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// This is where scripts' loading functions should be declared:
void AddSC_warlock_demonic_empowerment();
void AddSC_warlock_felguard_mandate();
void AddSC_warlock_crimson_shade();
void AddSC_warlock_corrupted_blood();
void AddSC_warlock_wrath_of_chaos();
void AddSC_demons_commandscript();
void AddSC_endless_instances();
void AddSC_arcturus_trade_skills();

// The name of this function should match:
// void Add${NameOfDirectory}Scripts()
void AddCustomScripts()
{
    AddSC_warlock_demonic_empowerment();
    AddSC_warlock_felguard_mandate();
    AddSC_warlock_crimson_shade();
    AddSC_warlock_corrupted_blood();
    AddSC_warlock_wrath_of_chaos();
    AddSC_demons_commandscript();
    AddSC_endless_instances();
    AddSC_arcturus_trade_skills();
}
