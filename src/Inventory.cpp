#include "Inventory.h"

namespace yamc
{
	Inventory::Inventory()
	{
		memset(items, 0, sizeof(items));
		for (int i = 0; i < MaxHotbarItems; i++) {
			hotbarItems[i] = MaxItems;
		}
	}

	void Inventory::setItem(uint32_t index, uint32_t id, int8_t count)
	{
		items[index].count = count;
		items[index].id = id;
	}

	void Inventory::removeItem(uint32_t index)
	{
		items[index].count = 0;
		items[index].id = 0;
	}

	void Inventory::removeItemFromHotbar(uint32_t index)
	{
		hotbarItems[index] = MaxHotbarItems;
	}

	void Inventory::setHotbarItem(uint32_t slotIndex, uint32_t itemIndex)
	{
		hotbarItems[slotIndex] = itemIndex;
	}

	InventoryItem& Inventory::getItem(uint32_t index)
	{
		return items[index];
	}

	const InventoryItem& Inventory::getItem(uint32_t index) const
	{
		return items[index];
	}

	InventoryItem* Inventory::getHotbarItem(uint32_t slotIndex)
	{
		if (hotbarItems[slotIndex] >= MaxItems) {
			return nullptr;
		}
		return &items[hotbarItems[slotIndex]];
	}

	const InventoryItem* Inventory::getHotbarItem(uint32_t slotIndex) const
	{
		if (hotbarItems[slotIndex] >= MaxItems) {
			return nullptr;
		}
		return &items[hotbarItems[slotIndex]];
	}


}