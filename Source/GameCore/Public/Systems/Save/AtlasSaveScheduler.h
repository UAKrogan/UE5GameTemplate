#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"

/*
 * Priority of a queued save request. Higher values win when the scheduler
 * pops the next request: Manual > Event > Autosave.
 */
enum class EAtlasSavePriority : uint8
{
	// Timer-driven background save into the autosave ring buffer.
	Autosave = 0,
	// Gameplay-event save (checkpoint, quest turn-in); debounced.
	Event = 1,
	// Player-initiated save; always processed first.
	Manual = 2
};

/*
 * One queued save request as produced by the scheduler and consumed by
 * FAtlasSaveSystem::ProcessNextSaveRequest.
 */
struct GAMECORE_API FAtlasScheduledSaveRequest
{
	// Destination slot; for autosaves this is a reserved Autosave_N slot.
	FString SlotName;

	EAtlasSavePriority Priority = EAtlasSavePriority::Autosave;

	// Ring-buffer index reserved for this request; INDEX_NONE for non-autosaves.
	int32 AutosaveIndex = INDEX_NONE;

	bool bAutosave = false;
	bool bEventSave = false;
};

/*
 * Tuning knobs for the scheduler, provided by the save system on Initialize.
 */
struct GAMECORE_API FAtlasSaveSchedulerConfig
{
	// Seconds between timer-driven autosaves.
	float AutosaveIntervalSeconds = 300.0f;

	// Window in which repeated event-save requests collapse into one.
	float EventDebounceSeconds = 2.0f;

	// Disable to make autosaves purely on-demand (RequestAutosave only).
	bool bEnableTimerAutosave = true;
};

/*
 * Schedules save requests for FAtlasSaveSystem.
 *
 * Responsibilities:
 * - Timer-based autosaves
 * - Event-triggered saves with debounce
 * - Priority ordering: Manual > Event > Autosave
 * - Queueing without starting multiple saves directly
 */
class GAMECORE_API FAtlasSaveScheduler
{
public:
	/*
	 * Invoked whenever a request lands in the queue so the owning save
	 * system can start processing on the game thread.
	 */
	using FOnSaveQueued = TFunction<void()>;

	/*
	 * Starts the ticker (for timer autosaves and event debounce) and stores
	 * the queued-callback. Must be paired with Shutdown().
	 */
	void Initialize(const FAtlasSaveSchedulerConfig& InConfig, FOnSaveQueued InOnSaveQueued);
	void Shutdown();

	/*
	 * Queues a player-initiated save for the given slot. Highest priority.
	 */
	bool RequestManualSave(const FString& SlotName);

	/*
	 * Queues a gameplay-event save; requests inside the debounce window
	 * collapse into a single pending request.
	 */
	bool RequestEventSave(const FString& SlotName);

	/*
	 * Reserves the next autosave ring-buffer slot and queues a save into it.
	 */
	bool RequestAutosave();

	/*
	 * Pops the highest-priority pending request. Returns false when empty.
	 */
	bool PopNextRequest(FAtlasScheduledSaveRequest& OutRequest);

	bool HasPendingRequests() const;
	int32 GetPendingRequestCount() const;

private:
	bool EnqueueRequest(FAtlasScheduledSaveRequest&& Request);
	bool Tick(float DeltaTime);
	void StartAutosaveTimer();
	void StopAutosaveTimer();
	double GetCurrentTimeSeconds() const;
	bool HasPendingEventSave() const;

	FAtlasSaveSchedulerConfig Config;
	FOnSaveQueued OnSaveQueued;
	FTSTicker::FDelegateHandle TickerHandle;
	TArray<FAtlasScheduledSaveRequest> PendingRequests;
	double LastAcceptedEventSaveTimeSeconds = -DBL_MAX;
	float AutosaveAccumulatorSeconds = 0.0f;
	bool bInitialized = false;
};
