--[[
  ArcturusDebug — live client-side debug log for custom warlock content.

  Slash: /adebug  (aliases: /ad, /arcturusdebug)
    on|off          enable / silence logging
    show|hide       toggle the log window
    clear           wipe the in-memory + SavedVariables ring buffer
    dump [n]        print last n lines to chat (default 30)
    filter <tag>    only show TAG (ITEM/SPELL/COMBAT/SYS/ICON/UI) or "all"
    probe [id]      snapshot GetItemInfo for custom entry (default scan 900001-900138)
    status          print current settings

  Agents: after /reload or logout, read:
    <client>/WTF/Account/<ACCOUNT>/SavedVariables/ArcturusDebug.lua
  Ring buffer is ArcturusDebugDB.lines (newest at end).
]]

local ADDON = "ArcturusDebug"
local MAX_LINES = 400
local CUSTOM_LO = 900001
local CUSTOM_HI = 900200

local playerGUID
local ui
local scrollChild
local lineWidgets = {}

local DEFAULTS = {
  enabled = true,
  showWindow = true,
  filter = "all", -- all | ITEM | SPELL | COMBAT | SYS | ICON | UI | ERR
  chatEcho = false, -- also spam DEFAULT_CHAT_FRAME (noisy)
}

local TAG_COLOR = {
  ITEM   = "ff66ccff",
  SPELL  = "ffffcc66",
  COMBAT = "ffff6666",
  SYS    = "ffc0a0ff",
  ICON   = "ff88ff88",
  UI     = "ffaaaaaa",
  ERR    = "ffff4444",
  INFO   = "ffffffff",
}

---------------------------------------------------------------------------
-- SavedVariables / logging core
---------------------------------------------------------------------------

local function EnsureDB()
  if type(ArcturusDebugDB) ~= "table" then
    ArcturusDebugDB = {}
  end
  local db = ArcturusDebugDB
  if type(db.settings) ~= "table" then
    db.settings = {}
  end
  for k, v in pairs(DEFAULTS) do
    if db.settings[k] == nil then
      db.settings[k] = v
    end
  end
  if type(db.lines) ~= "table" then
    db.lines = {}
  end
  return db
end

local function Settings()
  return EnsureDB().settings
end

local function NowStamp()
  -- HH:MM:SS.mmm relative to GetTime for ordering within a second
  local t = time()
  local ms = math.floor((GetTime() % 1) * 1000)
  return date("%H:%M:%S", t) .. string.format(".%03d", ms)
end

local function IsCustomItemId(id)
  id = tonumber(id)
  return id and id >= CUSTOM_LO and id <= CUSTOM_HI
end

local function ItemIdFromLink(link)
  if not link then
    return nil
  end
  local id = link:match("item:(%d+)")
  return id and tonumber(id) or nil
end

local function Truncate(s, n)
  s = tostring(s or "")
  if #s > n then
    return s:sub(1, n - 3) .. "..."
  end
  return s
end

local function RefreshWindow()
  if not ui or not scrollChild then
    return
  end
  local db = EnsureDB()
  local filter = Settings().filter or "all"
  local shown = {}
  for i = 1, #db.lines do
    local e = db.lines[i]
    if filter == "all" or e.tag == filter then
      shown[#shown + 1] = e
    end
  end

  local need = #shown
  while #lineWidgets < need do
    local fs = scrollChild:CreateFontString(nil, "ARTWORK", "GameFontHighlightSmall")
    fs:SetJustifyH("LEFT")
    fs:SetWidth(520)
    fs:SetNonSpaceWrap(true)
    if #lineWidgets == 0 then
      fs:SetPoint("TOPLEFT", scrollChild, "TOPLEFT", 2, -2)
    else
      fs:SetPoint("TOPLEFT", lineWidgets[#lineWidgets], "BOTTOMLEFT", 0, -1)
    end
    lineWidgets[#lineWidgets + 1] = fs
  end

  local height = 4
  for i = 1, #lineWidgets do
    local fs = lineWidgets[i]
    local e = shown[i]
    if e then
      local col = TAG_COLOR[e.tag] or TAG_COLOR.INFO
      fs:SetText(string.format("|cff%s[%s] %s|r |cffffffff%s|r", col, e.stamp, e.tag, e.msg))
      fs:Show()
      height = height + fs:GetStringHeight() + 1
    else
      fs:SetText("")
      fs:Hide()
    end
  end
  scrollChild:SetHeight(math.max(height, 1))
  if ui.scroll then
    ui.scroll:UpdateScrollChildRect()
    -- stick to bottom
    local max = math.max(0, height - (ui.body and ui.body:GetHeight() or 200))
    ui.scroll:SetVerticalScroll(max)
  end
  if ui.count then
    ui.count:SetText(string.format("%d / %d", #shown, #db.lines))
  end
end

local function Log(tag, msg, extra)
  local db = EnsureDB()
  if not Settings().enabled and tag ~= "INFO" and tag ~= "ERR" then
    return
  end

  local entry = {
    stamp = NowStamp(),
    time = time(),
    gettime = GetTime(),
    tag = tag or "INFO",
    msg = Truncate(msg, 400),
  }
  if type(extra) == "table" then
    entry.extra = extra
  end

  db.lines[#db.lines + 1] = entry
  while #db.lines > MAX_LINES do
    table.remove(db.lines, 1)
  end

  if Settings().chatEcho and DEFAULT_CHAT_FRAME then
    local col = TAG_COLOR[entry.tag] or TAG_COLOR.INFO
    DEFAULT_CHAT_FRAME:AddMessage(string.format("|cff%s[AD:%s]|r %s", col, entry.tag, entry.msg))
  end

  if Settings().showWindow then
    RefreshWindow()
  end
end

---------------------------------------------------------------------------
-- UI
---------------------------------------------------------------------------

local function CreateUI()
  if ui then
    return ui
  end

  local f = CreateFrame("Frame", "ArcturusDebugFrame", UIParent)
  f:SetWidth(560)
  f:SetHeight(280)
  f:SetPoint("CENTER", UIParent, "CENTER", 0, 120)
  f:SetBackdrop({
    bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background",
    edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border",
    tile = true, tileSize = 32, edgeSize = 16,
    insets = { left = 5, right = 5, top = 5, bottom = 5 },
  })
  f:SetBackdropColor(0, 0, 0, 0.85)
  f:SetMovable(true)
  f:EnableMouse(true)
  f:RegisterForDrag("LeftButton")
  f:SetScript("OnDragStart", f.StartMoving)
  f:SetScript("OnDragStop", f.StopMovingOrSizing)
  f:SetFrameStrata("HIGH")
  f:Hide()

  local title = f:CreateFontString(nil, "OVERLAY", "GameFontNormal")
  title:SetPoint("TOPLEFT", 14, -12)
  title:SetText("|cff9370dbArcturusDebug|r  /adebug")

  local count = f:CreateFontString(nil, "OVERLAY", "GameFontHighlightSmall")
  count:SetPoint("TOPRIGHT", -36, -14)
  count:SetText("0 / 0")

  local close = CreateFrame("Button", nil, f, "UIPanelCloseButton")
  close:SetPoint("TOPRIGHT", -2, -2)
  close:SetScript("OnClick", function()
    Settings().showWindow = false
    f:Hide()
  end)

  local body = CreateFrame("Frame", nil, f)
  body:SetPoint("TOPLEFT", 12, -32)
  body:SetPoint("BOTTOMRIGHT", -30, 36)

  local scroll = CreateFrame("ScrollFrame", "ArcturusDebugScroll", body, "UIPanelScrollFrameTemplate")
  scroll:SetPoint("TOPLEFT", body, "TOPLEFT", 0, 0)
  scroll:SetPoint("BOTTOMRIGHT", body, "BOTTOMRIGHT", -4, 0)

  local child = CreateFrame("Frame", nil, scroll)
  child:SetWidth(520)
  child:SetHeight(1)
  scroll:SetScrollChild(child)

  local function MakeBtn(text, x, onClick)
    local b = CreateFrame("Button", nil, f, "UIPanelButtonTemplate")
    b:SetWidth(70)
    b:SetHeight(22)
    b:SetPoint("BOTTOMLEFT", 12 + x, 10)
    b:SetText(text)
    b:SetScript("OnClick", onClick)
    return b
  end

  MakeBtn("Clear", 0, function()
    EnsureDB().lines = {}
    Log("INFO", "buffer cleared")
    RefreshWindow()
  end)
  MakeBtn("Dump", 78, function()
    SlashCmdList.ARCTURUSDEBUG("dump 20")
  end)
  MakeBtn("Probe", 156, function()
    SlashCmdList.ARCTURUSDEBUG("probe")
  end)
  MakeBtn("Echo", 234, function()
    local s = Settings()
    s.chatEcho = not s.chatEcho
    Log("UI", "chatEcho=" .. tostring(s.chatEcho))
  end)

  local hint = f:CreateFontString(nil, "OVERLAY", "GameFontDisableSmall")
  hint:SetPoint("BOTTOMRIGHT", -12, 14)
  hint:SetText("SavedVariables flush on /reload")

  ui = f
  ui.scroll = scroll
  ui.body = body
  ui.count = count
  scrollChild = child
  return f
end

local function ShowWindow(show)
  CreateUI()
  Settings().showWindow = show and true or false
  if show then
    ui:Show()
    RefreshWindow()
  else
    ui:Hide()
  end
end

---------------------------------------------------------------------------
-- Custom item helpers
---------------------------------------------------------------------------

local function ProbeItem(entry)
  entry = tonumber(entry)
  if not entry then
    return
  end
  local name, link, quality, iLevel, reqLevel, class, subclass, maxStack, equipLoc, tex = GetItemInfo(entry)
  local isQ = false
  if tex and (tex:find("[Qq]uestion") or tex:find("INV_Misc_QuestionMark")) then
    isQ = true
  end
  Log("ICON", string.format("item:%d known=%s name=%s tex=%s qmark=%s ilvl=%s loc=%s",
    entry,
    tostring(name ~= nil),
    Truncate(name or "?", 40),
    Truncate(tex or "?", 48),
    tostring(isQ),
    tostring(iLevel or "?"),
    tostring(equipLoc or "?")),
    {
      entry = entry,
      name = name,
      texture = tex,
      questionMark = isQ,
      link = link,
      equipLoc = equipLoc,
      quality = quality,
    })
  if not name then
    -- force client cache request
    GameTooltip:SetOwner(UIParent, "ANCHOR_NONE")
    GameTooltip:SetHyperlink("item:" .. entry)
    GameTooltip:Hide()
  end
end

local function ScanEquippedCustoms()
  local found = {}
  for slot = 0, 19 do
    local link = GetInventoryItemLink("player", slot)
    local id = ItemIdFromLink(link)
    if IsCustomItemId(id) then
      found[#found + 1] = string.format("%d@%d", id, slot)
      local tex = GetInventoryItemTexture("player", slot)
      local isQ = tex and (tex:find("[Qq]uestion") or tex:find("INV_Misc_QuestionMark")) and true or false
      Log("ITEM", string.format("EQUIPPED slot=%d item=%d tex=%s qmark=%s link=%s",
        slot, id, Truncate(tex or "?", 40), tostring(isQ), Truncate(link or "?", 60)),
        { slot = slot, entry = id, texture = tex, questionMark = isQ })
    end
  end
  if #found == 0 then
    Log("ITEM", "no custom items currently equipped (900001-900200)")
  end
end

local function ScanBagsForCustoms()
  local count = 0
  for bag = 0, 4 do
    local slots = GetContainerNumSlots(bag)
    for slot = 1, slots do
      local link = GetContainerItemLink(bag, slot)
      local id = ItemIdFromLink(link)
      if IsCustomItemId(id) then
        count = count + 1
        local tex = GetContainerItemInfo(bag, slot)
        local isQ = false
        if type(tex) == "string" and (tex:find("[Qq]uestion") or tex:find("INV_Misc_QuestionMark")) then
          isQ = true
        end
        -- GetContainerItemInfo returns texture as first return in 3.3.5
        Log("ITEM", string.format("BAG bag=%d slot=%d item=%d qmark=%s %s",
          bag, slot, id, tostring(isQ), Truncate(link or "?", 50)),
          { bag = bag, slot = slot, entry = id, questionMark = isQ })
      end
    end
  end
  Log("ITEM", "bag custom count=" .. count)
end

---------------------------------------------------------------------------
-- Event handlers
---------------------------------------------------------------------------

local function OnSpellCast(event, unit, spell, rank, lineId, spellId)
  if unit ~= "player" then
    return
  end
  spellId = tonumber(spellId)
  local msg = string.format("%s spell=%s id=%s rank=%s",
    event:gsub("UNIT_SPELLCAST_", ""),
    Truncate(spell or "?", 40),
    tostring(spellId or "?"),
    tostring(rank or "?"))

  -- Annotate if an equipped custom might be involved (on-use decoys)
  local customs = {}
  for slot = 0, 19 do
    local id = ItemIdFromLink(GetInventoryItemLink("player", slot))
    if IsCustomItemId(id) then
      customs[#customs + 1] = id
    end
  end
  if #customs > 0 then
    msg = msg .. " equippedCustoms=[" .. table.concat(customs, ",") .. "]"
  end
  Log("SPELL", msg, { event = event, spell = spell, spellId = spellId, customs = customs })
end

local function InterestingCLEU(subevent)
  -- Keep the buffer readable: skip per-hit damage/heal spam.
  return subevent == "SPELL_CAST_SUCCESS"
    or subevent == "SPELL_CAST_START"
    or subevent == "SPELL_CAST_FAILED"
    or subevent == "SPELL_MISSED"
    or subevent == "SPELL_AURA_APPLIED"
    or subevent == "SPELL_SUMMON"
    or subevent == "SPELL_CREATE"
end

local function OnCombatLog(...)
  -- 3.3.5a: timestamp, subevent, sourceGUID, sourceName, sourceFlags, destGUID, destName, destFlags, ...
  local timestamp, subevent, sourceGUID, sourceName, _, destGUID, destName = ...
  if not InterestingCLEU(subevent) then
    return
  end
  if sourceGUID ~= playerGUID and destGUID ~= playerGUID then
    return
  end

  local spellId, spellName = select(8, ...)
  -- For SPELL_* the next args after destFlags are spellId, spellName, spellSchool
  -- select(8,...) from full ... : arg1=ts ... arg8 = spellId when standard SPELL prefix
  -- Actually: 1=ts 2=sub 3=sGUID 4=sName 5=sFlags 6=dGUID 7=dName 8=dFlags 9=spellId 10=spellName
  spellId = select(9, ...)
  spellName = select(10, ...)

  local failOrAmt = select(12, ...)
  local msg = string.format("%s src=%s dst=%s spell=%s(%s)",
    tostring(subevent),
    Truncate(sourceName or "?", 16),
    Truncate(destName or "?", 16),
    Truncate(spellName or "?", 28),
    tostring(spellId or "?"))

  if subevent == "SPELL_CAST_FAILED" then
    msg = msg .. " reason=" .. Truncate(tostring(failOrAmt), 40)
  end

  -- Always log player-sourced; only log incoming if aura/summon from custom-looking names
  if sourceGUID == playerGUID or subevent == "SPELL_AURA_APPLIED" or subevent == "SPELL_SUMMON" then
    Log("COMBAT", msg, {
      subevent = subevent,
      spellId = spellId,
      spellName = spellName,
      dest = destName,
    })
  end
end

local function OnSystemMessage(msg)
  if not msg or msg == "" then
    return
  end
  -- Always keep server script chatter (color codes / known keywords)
  local lower = msg:lower()
  local keep = msg:find("|cff")
    or lower:find("cinderfury")
    or lower:find("void")
    or lower:find("fel")
    or lower:find("doom")
    or lower:find("soul")
    or lower:find("demon")
    or lower:find("lantern")
    or lower:find("grimoire")
    or lower:find("bloodseal")
    or lower:find("inferno")
    or lower:find("detonation")
    or lower:find("empower")
    or lower:find("cooldown")
    or lower:find("noggenfogger")
  if keep then
    Log("SYS", Truncate(msg, 350))
  end
end

---------------------------------------------------------------------------
-- Hooks for item use (more reliable than CLEU for decoy Use spells)
---------------------------------------------------------------------------

local function HookItemUse()
  if not hooksecurefunc then
    return
  end
  hooksecurefunc("UseContainerItem", function(bag, slot)
    local link = GetContainerItemLink(bag, slot)
    local id = ItemIdFromLink(link)
    if IsCustomItemId(id) then
      Log("ITEM", string.format("USE_BAG bag=%d slot=%d item=%d %s", bag, slot, id, Truncate(link or "?", 50)),
        { bag = bag, slot = slot, entry = id })
    end
  end)
  hooksecurefunc("UseInventoryItem", function(slot)
    local link = GetInventoryItemLink("player", slot)
    local id = ItemIdFromLink(link)
    if IsCustomItemId(id) then
      Log("ITEM", string.format("USE_INV slot=%d item=%d %s", slot, id, Truncate(link or "?", 50)),
        { slot = slot, entry = id })
    end
  end)
end

---------------------------------------------------------------------------
-- Slash commands
---------------------------------------------------------------------------

SLASH_ARCTURUSDEBUG1 = "/adebug"
SLASH_ARCTURUSDEBUG2 = "/ad"
SLASH_ARCTURUSDEBUG3 = "/arcturusdebug"

SlashCmdList["ARCTURUSDEBUG"] = function(msg)
  msg = strtrim(msg or "")
  local cmd, rest = msg:match("^(%S+)%s*(.*)$")
  cmd = cmd and cmd:lower() or "status"

  if cmd == "on" then
    Settings().enabled = true
    Log("INFO", "logging ON")
    ShowWindow(true)
  elseif cmd == "off" then
    Settings().enabled = false
    Log("INFO", "logging OFF")
  elseif cmd == "show" then
    ShowWindow(true)
  elseif cmd == "hide" then
    ShowWindow(false)
  elseif cmd == "clear" then
    EnsureDB().lines = {}
    Log("INFO", "buffer cleared")
    RefreshWindow()
  elseif cmd == "dump" then
    local n = tonumber(rest) or 30
    local db = EnsureDB()
    local start = math.max(1, #db.lines - n + 1)
    if DEFAULT_CHAT_FRAME then
      DEFAULT_CHAT_FRAME:AddMessage("|cff9370db[ArcturusDebug]|r dump " .. (#db.lines - start + 1) .. " lines:")
    end
    for i = start, #db.lines do
      local e = db.lines[i]
      if DEFAULT_CHAT_FRAME then
        DEFAULT_CHAT_FRAME:AddMessage(string.format("|cff888888%s|r %s %s", e.stamp, e.tag, e.msg))
      end
    end
  elseif cmd == "filter" then
    rest = (rest ~= "" and rest:upper()) or "ALL"
    if rest == "ALL" then
      Settings().filter = "all"
    else
      Settings().filter = rest
    end
    Log("UI", "filter=" .. Settings().filter)
    RefreshWindow()
  elseif cmd == "probe" then
    if rest ~= "" then
      ProbeItem(rest)
    else
      Log("ICON", "probing known customs 900001-900017 + sample expansion…")
      for _, id in ipairs({
        900001, 900002, 900006, 900008, 900011, 900016, 900017,
        900029, 900071, 900110, 900133, 900134, 900135, 900136, 900137, 900138,
      }) do
        ProbeItem(id)
      end
      ScanEquippedCustoms()
      ScanBagsForCustoms()
    end
  elseif cmd == "scan" then
    ScanEquippedCustoms()
    ScanBagsForCustoms()
  elseif cmd == "echo" then
    Settings().chatEcho = not Settings().chatEcho
    Log("UI", "chatEcho=" .. tostring(Settings().chatEcho))
  elseif cmd == "status" or cmd == "help" then
    local s = Settings()
    local db = EnsureDB()
    local text = string.format(
      "enabled=%s window=%s filter=%s chatEcho=%s lines=%d max=%d",
      tostring(s.enabled), tostring(s.showWindow), tostring(s.filter),
      tostring(s.chatEcho), #db.lines, MAX_LINES)
    Log("INFO", text)
    if DEFAULT_CHAT_FRAME then
      DEFAULT_CHAT_FRAME:AddMessage("|cff9370db[ArcturusDebug]|r " .. text)
      DEFAULT_CHAT_FRAME:AddMessage("|cff9370db[ArcturusDebug]|r /adebug on|off|show|hide|clear|dump|filter|probe|scan|echo")
    end
  else
    if DEFAULT_CHAT_FRAME then
      DEFAULT_CHAT_FRAME:AddMessage("|cff9370db[ArcturusDebug]|r unknown cmd. Try /adebug help")
    end
  end
end

---------------------------------------------------------------------------
-- Bootstrap
---------------------------------------------------------------------------

local invDebounce = CreateFrame("Frame")
invDebounce:Hide()
invDebounce.elapsed = 0
invDebounce:SetScript("OnUpdate", function(frame, dt)
  frame.elapsed = frame.elapsed + dt
  if frame.elapsed < 0.35 then
    return
  end
  frame:Hide()
  frame.elapsed = 0
  if Settings().enabled then
    Log("ITEM", "inventory changed — rescan equipped customs")
    ScanEquippedCustoms()
  end
end)

local driver = CreateFrame("Frame")
driver:RegisterEvent("ADDON_LOADED")
driver:RegisterEvent("PLAYER_LOGIN")
driver:RegisterEvent("PLAYER_ENTERING_WORLD")
driver:RegisterEvent("UNIT_SPELLCAST_START")
driver:RegisterEvent("UNIT_SPELLCAST_SUCCEEDED")
driver:RegisterEvent("UNIT_SPELLCAST_FAILED")
driver:RegisterEvent("UNIT_SPELLCAST_INTERRUPTED")
driver:RegisterEvent("UNIT_SPELLCAST_STOP")
driver:RegisterEvent("COMBAT_LOG_EVENT_UNFILTERED")
driver:RegisterEvent("CHAT_MSG_SYSTEM")
driver:RegisterEvent("UNIT_INVENTORY_CHANGED")
driver:RegisterEvent("PLAYER_REGEN_DISABLED")
driver:RegisterEvent("PLAYER_REGEN_ENABLED")

driver:SetScript("OnEvent", function(self, event, ...)
  if event == "ADDON_LOADED" then
    local name = ...
    if name ~= ADDON then
      return
    end
    EnsureDB()
    HookItemUse()
    return
  end

  if event == "PLAYER_LOGIN" then
    playerGUID = UnitGUID("player")
    CreateUI()
    Log("INFO", string.format("login %s-%s build=%s guid=%s",
      UnitName("player") or "?",
      GetRealmName() or "?",
      tostring(select(4, GetBuildInfo()) or "?"),
      Truncate(playerGUID or "?", 24)))
    if Settings().showWindow and Settings().enabled then
      ShowWindow(true)
    end
    -- deferred equip + icon probe
    local elapsed = 0
    self:SetScript("OnUpdate", function(frame, dt)
      elapsed = elapsed + dt
      if elapsed >= 2 then
        frame:SetScript("OnUpdate", nil)
        ScanEquippedCustoms()
        ProbeItem(900016)
        ProbeItem(900017)
        Log("INFO", "ready — reproduce the bug, then /adebug dump or /reload for SavedVariables")
      end
    end)
    return
  end

  if event == "PLAYER_ENTERING_WORLD" then
    Log("UI", "PLAYER_ENTERING_WORLD")
    return
  end

  if not Settings().enabled then
    return
  end

  if event:find("UNIT_SPELLCAST_") then
    OnSpellCast(event, ...)
  elseif event == "COMBAT_LOG_EVENT_UNFILTERED" then
    OnCombatLog(...)
  elseif event == "CHAT_MSG_SYSTEM" then
    OnSystemMessage(...)
  elseif event == "UNIT_INVENTORY_CHANGED" then
    local unit = ...
    if unit == "player" then
      invDebounce.elapsed = 0
      invDebounce:Show()
    end
  elseif event == "PLAYER_REGEN_DISABLED" then
    Log("UI", "combat START")
  elseif event == "PLAYER_REGEN_ENABLED" then
    Log("UI", "combat END")
  end
end)
