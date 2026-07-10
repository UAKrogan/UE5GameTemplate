#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AtlasPostLoadTrigger.generated.h"

/*
 * Placed in a map to signal "level is ready" to the level transition
 * subsystem, decoupling the signal from the game mode so different game
 * modes can be used per map.
 *
 * Lives in GameCore (not GameActors) because it exists solely to talk to
 * UAtlasLevelTransitionSubsystem and GameActors must not depend on GameCore.
 */
UCLASS(Blueprintable)
class GAMECORE_API AAtlasPostLoadTrigger : public AActor
{
	GENERATED_BODY()

public:
	AAtlasPostLoadTrigger();

protected:
	//~AActor interface
	virtual void BeginPlay() override;
	//~End of AActor interface

	/*
	 * Disable for levels that call NotifyLevelLoaded() manually (e.g. after
	 * async streaming completes).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atlas|Level Flow")
	bool bNotifyOnBeginPlay = true;
};
