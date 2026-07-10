#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AtlasInputConfigData.generated.h"

class UInputAction;
class UInputMappingContext;

/*
 * Maps an input action to the gameplay tag that activates abilities
 * (see Atlas.Input.Ability.*).
 */
USTRUCT(BlueprintType)
struct FAtlasAbilityInputBinding
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSoftObjectPtr<UInputAction> InputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (Categories = "Atlas.Input"))
	FGameplayTag InputTag;
};

/*
 * One input mapping context plus its ability bindings. Pawn data references
 * a list of these; the input extension component applies them on possession.
 *
 * Primary asset type: AtlasInputConfig.
 */
UCLASS(BlueprintType)
class GAMEACTORS_API UAtlasInputConfigData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType PrimaryAssetType;

	//~UPrimaryDataAsset interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	//~End of UPrimaryDataAsset interface

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	int32 Priority = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<FAtlasAbilityInputBinding> AbilityBindings;
};
