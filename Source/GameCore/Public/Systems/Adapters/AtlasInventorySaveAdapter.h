#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Systems/Savable/AtlasSavable.h"
#include "AtlasInventorySaveAdapter.generated.h"

USTRUCT(BlueprintType)
struct GAMECORE_API FAtlasInventoryItemSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FName ItemId;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FString ItemClassPath;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TMap<FName, FString> CustomData;

	bool IsValid() const
	{
		return !ItemId.IsNone() && Quantity > 0;
	}
};

/*
 * Generic inventory save adapter.
 *
 * This component owns a portable item array and serializes only that local data.
 * It has no dependency on FAtlasSaveSystem.
 */
UCLASS(ClassGroup = (Atlas), meta = (BlueprintSpawnableComponent))
class GAMECORE_API UAtlasInventorySaveAdapter : public UActorComponent, public IAtlasSavable
{
	GENERATED_BODY()

public:
	UAtlasInventorySaveAdapter();

	virtual void CaptureState(FAtlasSaveContext& Context) override;
	virtual void RestoreState(const FAtlasLoadContext& Context) override;

	const TArray<FAtlasInventoryItemSaveData>& GetItems() const;
	TArray<FAtlasInventoryItemSaveData>& GetMutableItems();
	void SetItems(const TArray<FAtlasInventoryItemSaveData>& InItems);
	void ClearItems();

private:
	void SerializeItem(FArchive& Archive, FAtlasInventoryItemSaveData& Item) const;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TArray<FAtlasInventoryItemSaveData> Items;
};
