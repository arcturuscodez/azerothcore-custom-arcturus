-- ArcturusItemFix
-- Custom item entries (900xxx) are absent from client Item.dbc, so the stock
-- right-click auto-equip path fails with EQUIP_ERR_ITEM_NOT_FOUND even though
-- drag-to-slot works (server knows the item). This hooks UseContainerItem and
-- equips via AutoEquipCursorItem using InventoryType from GetItemInfo.

local CUSTOM_MIN = 900001
local CUSTOM_MAX = 900199

local function IsArcturusCustom(itemId)
    return itemId and itemId >= CUSTOM_MIN and itemId <= CUSTOM_MAX
end

local function TryEquipFromBag(bag, slot)
    if CursorHasItem() then
        return false
    end

    local itemId = GetContainerItemID(bag, slot)
    if not IsArcturusCustom(itemId) then
        return false
    end

    local name, _, _, _, _, _, _, _, equipLoc = GetItemInfo(itemId)
    -- No cache yet, or not an equippable item (future consumables, etc.).
    if not name or not equipLoc or equipLoc == "" then
        return false
    end

    PickupContainerItem(bag, slot)
    if not CursorHasItem() then
        return false
    end

    AutoEquipCursorItem()
    -- If AutoEquip left it on the cursor (no valid slot), put it back.
    if CursorHasItem() then
        PickupContainerItem(bag, slot)
        return false
    end

    return true
end

local origUseContainerItem = UseContainerItem
function UseContainerItem(bag, slot, onSelf)
    if TryEquipFromBag(bag, slot) then
        return
    end
    return origUseContainerItem(bag, slot, onSelf)
end

-- Bank bags also call UseContainerItem in 3.3.5; covered by the same hook.

if DEFAULT_CHAT_FRAME then
    DEFAULT_CHAT_FRAME:AddMessage("|cff9370dbArcturusItemFix|r: custom item right-click equip enabled.")
end
