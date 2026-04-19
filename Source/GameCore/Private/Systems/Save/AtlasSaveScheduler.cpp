#include "Systems/Save/AtlasSaveScheduler.h"

#include "Logging/AtlasLogMacros.h"
#include "Systems/Save/AtlasAutosaveManager.h"

#include "Containers/Ticker.h"
#include "HAL/PlatformTime.h"

void FAtlasSaveScheduler::Initialize(const FAtlasSaveSchedulerConfig& InConfig, FOnSaveQueued InOnSaveQueued)
{
	Shutdown();

	Config = InConfig;
	OnSaveQueued = MoveTemp(InOnSaveQueued);
	PendingRequests.Reset();
	LastAcceptedEventSaveTimeSeconds = -DBL_MAX;
	AutosaveAccumulatorSeconds = 0.0f;
	bInitialized = true;

	StartAutosaveTimer();

	ATLAS_LOG_CORE(Log, "SaveScheduler initialized: autosaveEnabled=%s autosaveInterval=%.2f debounce=%.2f",
		Config.bEnableTimerAutosave ? TEXT("true") : TEXT("false"),
		Config.AutosaveIntervalSeconds,
		Config.EventDebounceSeconds);
}

void FAtlasSaveScheduler::Shutdown()
{
	StopAutosaveTimer();

	PendingRequests.Reset();
	OnSaveQueued = nullptr;
	bInitialized = false;
	AutosaveAccumulatorSeconds = 0.0f;
	LastAcceptedEventSaveTimeSeconds = -DBL_MAX;
}

bool FAtlasSaveScheduler::RequestManualSave(const FString& SlotName)
{
	FAtlasScheduledSaveRequest Request;
	Request.SlotName = SlotName;
	Request.Priority = EAtlasSavePriority::Manual;

	return EnqueueRequest(MoveTemp(Request));
}

bool FAtlasSaveScheduler::RequestEventSave(const FString& SlotName)
{
	const double CurrentTimeSeconds = GetCurrentTimeSeconds();
	const double SecondsSinceLastAcceptedEvent = CurrentTimeSeconds - LastAcceptedEventSaveTimeSeconds;

	if (SecondsSinceLastAcceptedEvent < Config.EventDebounceSeconds || HasPendingEventSave())
	{
		ATLAS_LOG_CORE(Log, "Event save debounced: slot=%s secondsSinceLast=%.2f pendingEvent=%s",
			*SlotName,
			SecondsSinceLastAcceptedEvent,
			HasPendingEventSave() ? TEXT("true") : TEXT("false"));
		return false;
	}

	FAtlasScheduledSaveRequest Request;
	Request.SlotName = SlotName;
	Request.Priority = EAtlasSavePriority::Event;
	Request.bEventSave = true;

	if (!EnqueueRequest(MoveTemp(Request)))
	{
		return false;
	}

	LastAcceptedEventSaveTimeSeconds = CurrentTimeSeconds;
	return true;
}

bool FAtlasSaveScheduler::RequestAutosave()
{
	FString AutosaveSlotName;
	int32 AutosaveIndex = INDEX_NONE;
	if (!FAtlasAutosaveManager::ReserveNextAutosaveSlot(AutosaveSlotName, AutosaveIndex))
	{
		ATLAS_LOG_CORE(Error, "Autosave request rejected: failed to reserve autosave slot");
		return false;
	}

	FAtlasScheduledSaveRequest Request;
	Request.SlotName = AutosaveSlotName;
	Request.Priority = EAtlasSavePriority::Autosave;
	Request.AutosaveIndex = AutosaveIndex;
	Request.bAutosave = true;

	return EnqueueRequest(MoveTemp(Request));
}

bool FAtlasSaveScheduler::PopNextRequest(FAtlasScheduledSaveRequest& OutRequest)
{
	if (PendingRequests.IsEmpty())
	{
		return false;
	}

	int32 BestRequestIndex = 0;
	for (int32 RequestIndex = 1; RequestIndex < PendingRequests.Num(); ++RequestIndex)
	{
		if (static_cast<uint8>(PendingRequests[RequestIndex].Priority) > static_cast<uint8>(PendingRequests[BestRequestIndex].Priority))
		{
			BestRequestIndex = RequestIndex;
		}
	}

	OutRequest = MoveTemp(PendingRequests[BestRequestIndex]);
	PendingRequests.RemoveAt(BestRequestIndex);
	return true;
}

bool FAtlasSaveScheduler::HasPendingRequests() const
{
	return !PendingRequests.IsEmpty();
}

int32 FAtlasSaveScheduler::GetPendingRequestCount() const
{
	return PendingRequests.Num();
}

bool FAtlasSaveScheduler::EnqueueRequest(FAtlasScheduledSaveRequest&& Request)
{
	check(IsInGameThread());

	if (!bInitialized)
	{
		ATLAS_LOG_CORE(Warning, "SaveScheduler rejected request before initialization: slot=%s", *Request.SlotName);
		return false;
	}

	if (Request.SlotName.IsEmpty())
	{
		ATLAS_LOG_CORE(Error, "SaveScheduler rejected request: slot name is empty");
		return false;
	}

	PendingRequests.Add(MoveTemp(Request));

	const FAtlasScheduledSaveRequest& QueuedRequest = PendingRequests.Last();
	ATLAS_LOG_CORE(Log, "SaveScheduler queued request: slot=%s priority=%d autosave=%s event=%s queueSize=%d",
		*QueuedRequest.SlotName,
		static_cast<int32>(QueuedRequest.Priority),
		QueuedRequest.bAutosave ? TEXT("true") : TEXT("false"),
		QueuedRequest.bEventSave ? TEXT("true") : TEXT("false"),
		PendingRequests.Num());

	if (OnSaveQueued)
	{
		OnSaveQueued();
	}

	return true;
}

bool FAtlasSaveScheduler::Tick(float DeltaTime)
{
	if (!bInitialized || !Config.bEnableTimerAutosave || Config.AutosaveIntervalSeconds <= 0.0f)
	{
		return true;
	}

	AutosaveAccumulatorSeconds += DeltaTime;
	if (AutosaveAccumulatorSeconds >= Config.AutosaveIntervalSeconds)
	{
		AutosaveAccumulatorSeconds = 0.0f;
		ATLAS_LOG_CORE(Log, "Timer autosave triggered");
		RequestAutosave();
	}

	return true;
}

void FAtlasSaveScheduler::StartAutosaveTimer()
{
	if (!Config.bEnableTimerAutosave || Config.AutosaveIntervalSeconds <= 0.0f || TickerHandle.IsValid())
	{
		return;
	}

	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FAtlasSaveScheduler::Tick));
}

void FAtlasSaveScheduler::StopAutosaveTimer()
{
	if (!TickerHandle.IsValid())
	{
		return;
	}

	FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
	TickerHandle.Reset();
}

double FAtlasSaveScheduler::GetCurrentTimeSeconds() const
{
	return FPlatformTime::Seconds();
}

bool FAtlasSaveScheduler::HasPendingEventSave() const
{
	for (const FAtlasScheduledSaveRequest& Request : PendingRequests)
	{
		if (Request.bEventSave)
		{
			return true;
		}
	}

	return false;
}
