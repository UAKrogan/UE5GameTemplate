#include "Systems/Adapters/AtlasInventorySaveAdapter.h"

#include "Logging/AtlasLogMacros.h"

namespace AtlasInventorySaveAdapter
{
	constexpr int32 DataVersion = 1;
}

UAtlasInventorySaveAdapter::UAtlasInventorySaveAdapter()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
}

void UAtlasInventorySaveAdapter::CaptureState(FAtlasSaveContext& Context)
{
	FArchive& Archive = Context.GetArchive();

	int32 Version = AtlasInventorySaveAdapter::DataVersion;
	int32 ItemCount = Items.Num();

	Archive << Version;
	Archive << ItemCount;

	for (FAtlasInventoryItemSaveData Item : Items)
	{
		SerializeItem(Archive, Item);
	}

	ATLAS_LOG_CORE(Verbose, "Captured inventory adapter state: owner=%s items=%d", *GetNameSafe(GetOwner()), Items.Num());
}

void UAtlasInventorySaveAdapter::RestoreState(const FAtlasLoadContext& Context)
{
	FArchive& Archive = const_cast<FAtlasLoadContext&>(Context).GetArchive();

	int32 Version = 0;
	int32 ItemCount = 0;

	Archive << Version;
	Archive << ItemCount;

	if (Archive.IsError() || Version <= 0 || ItemCount < 0)
	{
		ATLAS_LOG_CORE(Error, "Failed to restore inventory adapter state: owner=%s", *GetNameSafe(GetOwner()));
		return;
	}

	Items.Reset(ItemCount);
	for (int32 ItemIndex = 0; ItemIndex < ItemCount; ++ItemIndex)
	{
		FAtlasInventoryItemSaveData Item;
		SerializeItem(Archive, Item);

		if (Item.IsValid())
		{
			Items.Add(MoveTemp(Item));
		}
	}

	ATLAS_LOG_CORE(Verbose, "Restored inventory adapter state: owner=%s items=%d", *GetNameSafe(GetOwner()), Items.Num());
}

const TArray<FAtlasInventoryItemSaveData>& UAtlasInventorySaveAdapter::GetItems() const
{
	return Items;
}

TArray<FAtlasInventoryItemSaveData>& UAtlasInventorySaveAdapter::GetMutableItems()
{
	return Items;
}

void UAtlasInventorySaveAdapter::SetItems(const TArray<FAtlasInventoryItemSaveData>& InItems)
{
	Items = InItems;
}

void UAtlasInventorySaveAdapter::ClearItems()
{
	Items.Reset();
}

void UAtlasInventorySaveAdapter::SerializeItem(FArchive& Archive, FAtlasInventoryItemSaveData& Item) const
{
	Archive << Item.ItemId;
	Archive << Item.ItemClassPath;
	Archive << Item.Quantity;
	Archive << Item.CustomData;
}
