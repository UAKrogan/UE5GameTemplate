#include "Subsystems/AtlasGameFlowSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Logging/AtlasLogMacros.h"
#include "Settings/AtlasDeveloperSettings.h"

void UAtlasGameFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<UAtlasLevelTransitionSubsystem>();

	if (UAtlasLevelTransitionSubsystem* Transitions = GetTransitionSubsystem())
	{
		Transitions->OnPostLoad.AddDynamic(this, &UAtlasGameFlowSubsystem::HandlePostLoad);
		Transitions->OnTransitionFailed.AddDynamic(this, &UAtlasGameFlowSubsystem::HandleTransitionFailed);
	}

	CurrentState = EAtlasGameFlowState::Startup;

	ATLAS_LOG_CORE(Log, "Game flow subsystem initialized (Startup)");
}

void UAtlasGameFlowSubsystem::Deinitialize()
{
	if (UAtlasLevelTransitionSubsystem* Transitions = GetTransitionSubsystem())
	{
		Transitions->OnPostLoad.RemoveDynamic(this, &UAtlasGameFlowSubsystem::HandlePostLoad);
		Transitions->OnTransitionFailed.RemoveDynamic(this, &UAtlasGameFlowSubsystem::HandleTransitionFailed);
	}

	Super::Deinitialize();
}

void UAtlasGameFlowSubsystem::StartGameFlow()
{
	if (CurrentState != EAtlasGameFlowState::Startup)
	{
		return;
	}

	const UAtlasDeveloperSettings* Settings = UAtlasDeveloperSettings::Get();

	const UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	const bool bIsPIE = World != nullptr && World->WorldType == EWorldType::PIE;

	if (!Settings->MainMenuMap.IsNull() && !bIsPIE)
	{
		FAtlasLevelTransitionRequest Request;
		Request.DestinationMap = Settings->MainMenuMap;
		Request.TransitionType = EAtlasTransitionType::ToMainMenu;

		PendingStateAfterTransition = EAtlasGameFlowState::MainMenu;
		EnterState(EAtlasGameFlowState::Transitioning);

		if (UAtlasLevelTransitionSubsystem* Transitions = GetTransitionSubsystem())
		{
			if (Transitions->RequestTransition(Request))
			{
				return;
			}
		}
	}

	// No menu map configured (or PIE): become the main menu in place so the
	// flow is still queryable and delegates fire.
	EnterState(EAtlasGameFlowState::MainMenu);
}

bool UAtlasGameFlowSubsystem::RequestGameplay(TSoftObjectPtr<UWorld> GameplayMap)
{
	if (CurrentState == EAtlasGameFlowState::Transitioning)
	{
		return false;
	}

	if (GameplayMap.IsNull())
	{
		GameplayMap = UAtlasDeveloperSettings::Get()->DefaultGameplayMap;
	}

	FAtlasLevelTransitionRequest Request;
	Request.DestinationMap = GameplayMap;
	Request.TransitionType = EAtlasTransitionType::ToGameplay;
	Request.bSaveCheckpointBeforeTravel = UAtlasDeveloperSettings::Get()->bSaveCheckpointOnTravel;

	UAtlasLevelTransitionSubsystem* Transitions = GetTransitionSubsystem();
	if (Transitions == nullptr)
	{
		return false;
	}

	PendingStateAfterTransition = EAtlasGameFlowState::Gameplay;
	EnterState(EAtlasGameFlowState::Transitioning);

	if (!Transitions->RequestTransition(Request))
	{
		EnterState(EAtlasGameFlowState::MainMenu);
		return false;
	}

	return true;
}

bool UAtlasGameFlowSubsystem::RequestReturnToMainMenu()
{
	if (CurrentState == EAtlasGameFlowState::Transitioning)
	{
		return false;
	}

	FAtlasLevelTransitionRequest Request;
	Request.DestinationMap = UAtlasDeveloperSettings::Get()->MainMenuMap;
	Request.TransitionType = EAtlasTransitionType::ToMainMenu;
	Request.bSaveCheckpointBeforeTravel = UAtlasDeveloperSettings::Get()->bSaveCheckpointOnTravel;

	UAtlasLevelTransitionSubsystem* Transitions = GetTransitionSubsystem();
	if (Transitions == nullptr)
	{
		return false;
	}

	const EAtlasGameFlowState PreviousState = CurrentState;
	PendingStateAfterTransition = EAtlasGameFlowState::MainMenu;
	EnterState(EAtlasGameFlowState::Transitioning);

	if (!Transitions->RequestTransition(Request))
	{
		EnterState(PreviousState);
		return false;
	}

	return true;
}

void UAtlasGameFlowSubsystem::RequestPause()
{
	if (CurrentState == EAtlasGameFlowState::Gameplay)
	{
		EnterState(EAtlasGameFlowState::Paused);
	}
}

void UAtlasGameFlowSubsystem::RequestResume()
{
	if (CurrentState == EAtlasGameFlowState::Paused)
	{
		EnterState(EAtlasGameFlowState::Gameplay);
	}
}

void UAtlasGameFlowSubsystem::EnterState(EAtlasGameFlowState NewState)
{
	if (NewState == CurrentState)
	{
		return;
	}

	const EAtlasGameFlowState OldState = CurrentState;

	OnPreEnterState(NewState);
	CurrentState = NewState;
	OnPostEnterState(NewState);

	OnGameFlowStateChanged.Broadcast(OldState, NewState);

	ATLAS_LOG_CORE(Log, "Game flow state: %d -> %d", static_cast<int32>(OldState), static_cast<int32>(NewState));
}

void UAtlasGameFlowSubsystem::HandlePostLoad(const FAtlasLevelTransitionRequest& Request)
{
	if (CurrentState == EAtlasGameFlowState::Transitioning)
	{
		EnterState(PendingStateAfterTransition);
	}
}

void UAtlasGameFlowSubsystem::HandleTransitionFailed(const FAtlasLevelTransitionRequest& Request, const FString& Reason)
{
	if (CurrentState == EAtlasGameFlowState::Transitioning)
	{
		EnterState(EAtlasGameFlowState::MainMenu);
	}
}

UAtlasLevelTransitionSubsystem* UAtlasGameFlowSubsystem::GetTransitionSubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UAtlasLevelTransitionSubsystem>() : nullptr;
}
