--[[
    ALE / Lua 5.2+ compatibility shims for Paragon Anniversary.

    Load order: ALE runs ScriptPath files alphabetically; this file is named
    000_* so it runs before paragon/**. Mediator also binds local unpack —
    keep both so any other Paragon script using global unpack still works.
]]

if table.unpack and not unpack then
    unpack = table.unpack
end
