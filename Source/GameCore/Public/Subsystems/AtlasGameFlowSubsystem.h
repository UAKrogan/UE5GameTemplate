#pragma once

#include "CoreMinimal.h"
#include "Subsystems/AtlasLevelTransitionSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AtlasGameFlowSubsystem.generated.h"

UENUM(BlueprintType)
enum class EAtlasGameFlowState : uint8
{
	// Boot state before any map is playable.
	Startup,
	// Main menu is active (with or without a dedicated menu map).
	MainMenu,
	// Gameplay is active.
	Gameplay,
	// Gameplay suspended by the player; resumable in place.
	Paused,
	// A level transition is in flight; the target state is applied on
	// post-load (or reverted on failure).
	Transitioning
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAtlasGameFlowStateChanged, EAtlasGameFlowState, OldState, EAtlasGameFlowState, NewState);

/*
 * Top-level game state machine: Startup -> MainMenu -> Gameplay -> Paused.
 *
 * Owns *what* state the game is in; the level transition subsystem owns
 * *how* maps change. Other systems react to OnGameFlowStateChanged.
 */
UCLASS()
class GAMECORE_API UAtlasGameFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	/*
	 * Kicks off the startup flow. Called by the game instance once the engine
	 * has started the game (UGameInstance::OnStart). Travels to the main menu
	 * map when one is configured; otherwise enters MainMenu state in place.
	 * Auto-travel is skipped in PIE so it never hijacks the map under edit.
	 */
	void StartGameFlow();

	UFUNCTION(BlueprintCallable, Category = "Atlas|Game Flow")
	EAtlasGameFlowState GetCurrentState() const { return CurrentState; }

	/*
	 * Travels to the given gameplay map (or the configured default when
	 * unset) and enters Gameplay on post-load.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Game Flow")
	bool RequestGameplay(TSoftObjectPtr<UWorld> GameplayMap);

	/*
	 * Travels back to the configured main menu map and enters MainMenu on
	 * post-load. Returns false when a transition is already in flight.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Game Flow")
	bool RequestReturnToMainMenu();

	/*
	 * Gameplay -> Paused state change. State only: pausing the world
	 * (SetGamePaused) and pushing the pause screen are up to listeners.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Game Flow")
	void RequestPause();

	/*
	 * Paused -> Gameplay state change.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Game Flow")
	void RequestResume();

	UPROPERTY(BlueprintAssignable, Category = "Atlas|Game Flow")
	FAtlasGameFlowStateChanged OnGameFlowStateChanged;

protected:
	/*
	 * Per-state hooks for game-project subclasses, called around every state
	 * change (before/after CurrentState is updated).
	 */
	virtual void OnPreEnterState(EAtlasGameFlowState NewState) {}
	virtual void OnPostEnterState(EAtlasGameFlowState NewState) {}

private:
	void EnterState(EAtlasGameFlowState NewState);
	UFUNCTION()
	void HandlePostLoad(const FAtlasLevelTransitionRequest& Request);
	UFUNCTION()
	void HandleTransitionFailed(const FAtlasLevelTransitionRequest& Request, const FString& Reason);
	UAtlasLevelTransitionSubsystem* GetTransitionSubsystem() const;

	EAtlasGameFlowState CurrentState = EAtlasGameFlowState::Startup;
	// State to enter once the in-flight transition's post-load fires.
	EAtlasGameFlowState PendingStateAfterTransition = EAtlasGameFlowState::MainMenu;
};
