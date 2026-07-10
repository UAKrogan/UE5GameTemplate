#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AtlasLevelTransitionSubsystem.generated.h"

class UWorld;

UENUM(BlueprintType)
enum class EAtlasTransitionType : uint8
{
	// From gameplay to main menu.
	ToMainMenu,
	// From main menu to gameplay.
	ToGameplay,
	// Restart the current gameplay level.
	RestartLevel,
	// Arbitrary soft-ref map (used by feature plugins).
	ToCustomMap
};

UENUM(BlueprintType)
enum class EAtlasLevelTransitionPhase : uint8
{
	Idle,
	PreTravel,
	Traveling,
	PostLoad
};

USTRUCT(BlueprintType)
struct GAMECORE_API FAtlasLevelTransitionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	TSoftObjectPtr<UWorld> DestinationMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	EAtlasTransitionType TransitionType = EAtlasTransitionType::ToGameplay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	FString TravelOptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	bool bShowLoadingScreen = true;

	/*
	 * When true, an event-priority checkpoint save is requested in the
	 * pre-travel phase. World state is collected synchronously on the game
	 * thread before travel; serialization and the file write finish
	 * asynchronously and are unaffected by the world teardown.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	bool bSaveCheckpointBeforeTravel = false;

	/*
	 * When set, this save slot is loaded (world state restored) right after
	 * the destination level's post-load phase.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	FString LoadSlotOnPostLoad;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAtlasLevelTransitionEvent, const FAtlasLevelTransitionRequest&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAtlasLevelTransitionFailed, const FAtlasLevelTransitionRequest&, Request, const FString&, Reason);

/*
 * The single authority for level travel.
 *
 * Enforces the transition lifecycle (PreTravel -> Traveling -> PostLoad),
 * prevents re-entrant transitions, and notifies observers via delegates.
 * Override PerformTravel() in a game project subclass for seamless travel.
 */
UCLASS()
class GAMECORE_API UAtlasLevelTransitionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	/*
	 * Requests a transition. Returns false if a transition is already in
	 * progress or the request is invalid.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Level Flow")
	bool RequestTransition(const FAtlasLevelTransitionRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "Atlas|Level Flow")
	bool IsTransitioning() const { return Phase != EAtlasLevelTransitionPhase::Idle; }

	UFUNCTION(BlueprintCallable, Category = "Atlas|Level Flow")
	EAtlasLevelTransitionPhase GetCurrentPhase() const { return Phase; }

	UFUNCTION(BlueprintCallable, Category = "Atlas|Level Flow")
	const FAtlasLevelTransitionRequest& GetPendingRequest() const { return PendingRequest; }

	/*
	 * Signals that the destination level is loaded and ready. Called
	 * automatically on PostLoadMapWithWorld; AAtlasPostLoadTrigger placed in
	 * a map provides an explicit, decoupled signal for the same thing.
	 * Idempotent outside the Traveling phase.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Level Flow")
	void NotifyLevelLoaded();

	// Fired before travel starts: save checkpoints, close UI, clean up.
	UPROPERTY(BlueprintAssignable, Category = "Atlas|Level Flow")
	FAtlasLevelTransitionEvent OnPreTravel;

	// Fired after the destination level is loaded and ready.
	UPROPERTY(BlueprintAssignable, Category = "Atlas|Level Flow")
	FAtlasLevelTransitionEvent OnPostLoad;

	UPROPERTY(BlueprintAssignable, Category = "Atlas|Level Flow")
	FAtlasLevelTransitionFailed OnTransitionFailed;

protected:
	/*
	 * Executes the actual travel. Default: non-seamless OpenLevel. Override
	 * for seamless/multiplayer travel — the pre/post lifecycle is identical.
	 */
	virtual void PerformTravel(const FAtlasLevelTransitionRequest& Request);

private:
	void HandlePostLoadMap(UWorld* LoadedWorld);
	void FailTransition(const FString& Reason);

	FAtlasLevelTransitionRequest PendingRequest;
	EAtlasLevelTransitionPhase Phase = EAtlasLevelTransitionPhase::Idle;
	FDelegateHandle PostLoadMapHandle;
};
