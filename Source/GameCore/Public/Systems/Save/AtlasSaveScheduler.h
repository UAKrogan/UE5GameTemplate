#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"

enum class EAtlasSavePriority : uint8
{
	Autosave = 0,
	Event = 1,
	Manual = 2
};

struct GAMECORE_API FAtlasScheduledSaveRequest
{
	FString SlotName;
	EAtlasSavePriority Priority = EAtlasSavePriority::Autosave;
	int32 AutosaveIndex = INDEX_NONE;
	bool bAutosave = false;
	bool bEventSave = false;
};

struct GAMECORE_API FAtlasSaveSchedulerConfig
{
	float AutosaveIntervalSeconds = 300.0f;
	float EventDebounceSeconds = 2.0f;
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
	using FOnSaveQueued = TFunction<void()>;

	void Initialize(const FAtlasSaveSchedulerConfig& InConfig, FOnSaveQueued InOnSaveQueued);
	void Shutdown();

	bool RequestManualSave(const FString& SlotName);
	bool RequestEventSave(const FString& SlotName);
	bool RequestAutosave();

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
