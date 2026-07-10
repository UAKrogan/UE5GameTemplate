#include "Subsystems/AtlasLevelTransitionSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Logging/AtlasLogMacros.h"
#include "Subsystems/AtlasGameInstanceSubsystem.h"
#include "Subsystems/AtlasLoadingScreenSubsystem.h"
#include "Systems/Load/IAtlasLoadSystem.h"
#include "Systems/Save/IAtlasSaveSystem.h"
#include "UObject/UObjectGlobals.h"

namespace AtlasLevelTransition
{
	const TCHAR* TravelCheckpointSlotName = TEXT("Checkpoint_Travel");
}

void UAtlasLevelTransitionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<UAtlasLoadingScreenSubsystem>();

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &UAtlasLevelTransitionSubsystem::HandlePostLoadMap);

	ATLAS_LOG_CORE(Log, "Level transition subsystem initialized");
}

void UAtlasLevelTransitionSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);

	Super::Deinitialize();
}

bool UAtlasLevelTransitionSubsystem::RequestTransition(const FAtlasLevelTransitionRequest& Request)
{
	if (IsTransitioning())
	{
		ATLAS_LOG_CORE(Warning, "Transition rejected: another transition is in progress (phase %d)",
			static_cast<int32>(Phase));
		return false;
	}

	if (Request.DestinationMap.IsNull())
	{
		ATLAS_LOG_CORE(Warning, "Transition rejected: destination map is not set");
		return false;
	}

	PendingRequest = Request;
	Phase = EAtlasLevelTransitionPhase::PreTravel;

	ATLAS_LOG_CORE(Log, "Transition requested: %s", *Request.DestinationMap.ToSoftObjectPath().ToString());

	OnPreTravel.Broadcast(PendingRequest);

	if (Request.bSaveCheckpointBeforeTravel)
	{
		// World state is collected synchronously on the game thread before
		// travel starts; serialization and the file write complete async.
		if (const UAtlasGameInstanceSubsystem* Systems = GetGameInstance()->GetSubsystem<UAtlasGameInstanceSubsystem>())
		{
			if (const TSharedPtr<IAtlasSaveSystem> SaveSystem = Systems->GetSystem<IAtlasSaveSystem>())
			{
				SaveSystem->RequestEventSave(AtlasLevelTransition::TravelCheckpointSlotName);
			}
		}
	}

	if (Request.bShowLoadingScreen)
	{
		if (UAtlasLoadingScreenSubsystem* LoadingScreen = GetGameInstance()->GetSubsystem<UAtlasLoadingScreenSubsystem>())
		{
			LoadingScreen->ShowLoadingScreen();
		}
	}

	Phase = EAtlasLevelTransitionPhase::Traveling;
	PerformTravel(PendingRequest);

	return true;
}

void UAtlasLevelTransitionSubsystem::NotifyLevelLoaded()
{
	if (Phase != EAtlasLevelTransitionPhase::Traveling)
	{
		return;
	}

	Phase = EAtlasLevelTransitionPhase::PostLoad;

	ATLAS_LOG_CORE(Log, "Transition post-load: %s", *PendingRequest.DestinationMap.ToSoftObjectPath().ToString());

	OnPostLoad.Broadcast(PendingRequest);

	if (!PendingRequest.LoadSlotOnPostLoad.IsEmpty())
	{
		if (const UAtlasGameInstanceSubsystem* Systems = GetGameInstance()->GetSubsystem<UAtlasGameInstanceSubsystem>())
		{
			if (const TSharedPtr<IAtlasLoadSystem> LoadSystem = Systems->GetSystem<IAtlasLoadSystem>())
			{
				LoadSystem->RequestLoad(PendingRequest.LoadSlotOnPostLoad);
			}
		}
	}

	if (PendingRequest.bShowLoadingScreen)
	{
		if (UAtlasLoadingScreenSubsystem* LoadingScreen = GetGameInstance()->GetSubsystem<UAtlasLoadingScreenSubsystem>())
		{
			LoadingScreen->HideLoadingScreen();
		}
	}

	Phase = EAtlasLevelTransitionPhase::Idle;
}

void UAtlasLevelTransitionSubsystem::PerformTravel(const FAtlasLevelTransitionRequest& Request)
{
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, Request.DestinationMap, /*bAbsolute*/ true, Request.TravelOptions);
}

void UAtlasLevelTransitionSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	NotifyLevelLoaded();
}

void UAtlasLevelTransitionSubsystem::FailTransition(const FString& Reason)
{
	ATLAS_LOG_CORE(Warning, "Transition failed: %s", *Reason);

	const FAtlasLevelTransitionRequest FailedRequest = PendingRequest;
	Phase = EAtlasLevelTransitionPhase::Idle;

	if (FailedRequest.bShowLoadingScreen)
	{
		if (UAtlasLoadingScreenSubsystem* LoadingScreen = GetGameInstance()->GetSubsystem<UAtlasLoadingScreenSubsystem>())
		{
			LoadingScreen->HideLoadingScreen();
		}
	}

	OnTransitionFailed.Broadcast(FailedRequest, Reason);
}
