#ifndef YAMC_INVENTORY_H
#define YAMC_INVENTORY_H

#include <stdint.h>
#include <vector>

namespace yamc
{


	struct InventoryItem
	{
		uint32_t id;
		int8_t count;
	};

	class Inventory
	{
	public:
		static constexpr uint32_t MaxHotbarItems = 9;
		static constexpr uint32_t MaxItems = 64;
		static constexpr int8_t Countless = -1;
		
		Inventory();
		void setItem(uint32_t index, uint32_t id, int8_t count = Countless);
		void removeItem(uint32_t index);
		void removeItemFromHotbar(uint32_t index);
		void setHotbarItem(uint32_t slotIndex, uint32_t itemIndex);

		InventoryItem& getItem(uint32_t index);
		const InventoryItem& getItem(uint32_t index) const;

		InventoryItem* getHotbarItem(uint32_t slotIndex);
		const InventoryItem* getHotbarItem(uint32_t slotIndex) const;

		uint32_t getSelectedHotbarSlot() const;
		void scrollHotbar(int delta);

	private:
		InventoryItem items[MaxItems];
		uint32_t hotbarItems[MaxHotbarItems];
		uint32_t selectedHotbarSlot;
	};
}

#endif