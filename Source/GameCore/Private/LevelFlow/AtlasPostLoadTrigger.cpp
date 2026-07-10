#include "LevelFlow/AtlasPostLoadTrigger.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Subsystems/AtlasLevelTransitionSubsystem.h"

AAtlasPostLoadTrigger::AAtlasPostLoadTrigger()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAtlasPostLoadTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (!bNotifyOnBeginPlay)
	{
		return;
	}

	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (UAtlasLevelTransitionSubsystem* Transitions = GameInstance->GetSubsystem<UAtlasLevelTransitionSubsystem>())
		{
			Transitions->NotifyLevelLoaded();
		}
	}
}
