-- ArcturusSpellFix — rewrite stock Inferno / Ritual of Doom tooltips.
-- Server makes both permanent pet summons but still consumes Infernal Stone /
-- Demonic Figurine. Client Spell.dbc still shows the vanilla group-ritual text
-- for Ritual of Doom until a Spell.dbc MPQ exists.

local SPELL_INFERNO = 1122
local SPELL_RITUAL_OF_DOOM = 18540

local TOOLTIPS = {
    [SPELL_INFERNO] = {
        name = "Inferno",
        lines = {
            "Summons an Infernal to fight for you.",
            "Requires Infernal Stone.",
            "The demon remains until dismissed or slain.",
            "Replaces your current demon.",
        },
    },
    [SPELL_RITUAL_OF_DOOM] = {
        name = "Summon Doomguard",
        lines = {
            "Summons a Doomguard to fight for you.",
            "Requires Demonic Figurine. No group ritual.",
            "The demon remains until dismissed or slain.",
            "Replaces your current demon.",
        },
    },
}

local function RewriteSpellTooltip(tooltip)
    local _, spellId = tooltip:GetSpell()
    local info = spellId and TOOLTIPS[spellId]
    if not info then
        return
    end

    local nameFS = _G[tooltip:GetName() .. "TextLeft1"]
    if nameFS then
        nameFS:SetText(info.name)
    end

    -- Leave line 2 (usually rank / cost) alone; replace body starting at line 3.
    local lineIndex = 3
    for i = 3, tooltip:NumLines() do
        local left = _G[tooltip:GetName() .. "TextLeft" .. i]
        if left then
            left:SetText("")
        end
    end

    for _, text in ipairs(info.lines) do
        local left = _G[tooltip:GetName() .. "TextLeft" .. lineIndex]
        if left then
            left:SetText(text)
            left:SetTextColor(1, 0.82, 0)
        end
        lineIndex = lineIndex + 1
    end

    -- Clear leftover vanilla group-ritual / cooldown lines further down.
    for i = lineIndex, tooltip:NumLines() do
        local left = _G[tooltip:GetName() .. "TextLeft" .. i]
        if left then
            local t = left:GetText() or ""
            if t:find("party") or t:find("Ritual") or t:find("participant")
                or t:find("cooldown") or t:find("Cooldown") then
                left:SetText("")
            end
        end
    end
end

local function HookTooltip(tt)
    if not tt or tt._arcturusSpellFix then
        return
    end
    tt._arcturusSpellFix = true
    tt:HookScript("OnTooltipSetSpell", RewriteSpellTooltip)
end

HookTooltip(GameTooltip)
HookTooltip(ItemRefTooltip)
if SpellBookTooltip then
    HookTooltip(SpellBookTooltip)
end
