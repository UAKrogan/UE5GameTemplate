#pragma once

#include "GameFeatureAction.h"
#include "AtlasGameFeatureAction_AddInputConfig.generated.h"

class AAtlasPlayerController;
class UAtlasInputConfigData;
struct FComponentRequestHandle;

/*
 * Applies additional input configs (mapping contexts + ability bindings) to
 * Atlas player controllers while the feature is active.
 */
UCLASS(meta = (DisplayName = "Atlas: Add Input Config"))
class GAMEACTORS_API UAtlasGameFeatureAction_AddInputConfig : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction interface
	virtual void OnGameFeatureActivating() override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~End of UGameFeatureAction interface

	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<TSoftObjectPtr<UAtlasInputConfigData>> InputConfigs;

private:
	void HandleActorExtension(AActor* Actor, FName EventName);
	void ApplyToController(AAtlasPlayerController* Controller);
	void RemoveFromController(AAtlasPlayerController* Controller);

	TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
	TSet<TWeakObjectPtr<AAtlasPlayerController>> AppliedControllers;
};
