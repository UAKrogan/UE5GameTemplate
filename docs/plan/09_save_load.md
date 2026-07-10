# 09 — Save/Load Architecture

---

## Current State

The save/load pipeline is the **most complete** system in the repository. The following is already implemented:

| Component | Status |
|---|---|
| `IAtlasSaveSystem` / `FAtlasSaveSystem` | Implemented |
| `IAtlasLoadSystem` / `FAtlasLoadSystem` | Implemented |
| `FAtlasSaveScheduler` (priority queue, debounce, timer) | Implemented |
| `FAtlasAutosaveManager` (5-slot ring buffer) | Implemented |
| `FAtlasBinaryWriter` / `FAtlasBinaryReader` | Implemented |
| `FAtlasSaveCollector` | Implemented |
| `FAtlasFileStorage` | Implemented |
| `FAtlasSaveGameSnapshot` (flat key/value store) | Implemented |
| `FAtlasWorldSnapshot` / `FAtlasActorSnapshot` / `FAtlasDataChunk` | Implemented |
| `FAtlasSaveContext` / `FAtlasLoadContext` | Implemented |
| `IAtlasSavable` / `UAtlasSavableComponent` | Implemented |
| `UAtlasGASSaveAdapter` | Implemented |
| `UAtlasInventorySaveAdapter` | Implemented (stub) |

This section describes **gaps and future work** only. Do not re-implement what exists.

---

## Gaps and Additions Needed

### 1. Async Threading Safety

**Current risk:** `FAtlasSaveSystem::bSaveInProgress` and `FAtlasLoadSystem::bLoadInProgress` are plain `bool`. If completion callbacks ever fire off the game thread, this is a race condition.

**Fix:** Change to `std::atomic<bool>` or `TAtomic<bool>`, or enforce that all callbacks are dispatched to the game thread via `AsyncTask(ENamedThreads::GameThread, ...)`.

---

### 2. Save Versioning and Migration

**Current state:** `FAtlasSaveHeader::CurrentVersion = 1` is written but there is no migration path when the version is incremented.

**Additions needed:**
```cpp
// In FAtlasSaveHeader or a new FAtlasSaveMigrationManager:
static bool MigrateSnapshot(FAtlasWorldSnapshot& Snapshot, int32 FromVersion, int32 ToVersion);

// Migration registry:
using FAtlasMigrationFn = TFunction<bool(FAtlasWorldSnapshot&)>;
static TMap<int32, FAtlasMigrationFn> Migrations;  // keyed by source version
```

**Recommended approach:** Version is stored in `FAtlasSaveGameMetadata::DataVersion`. On load, if `DataVersion < CurrentVersion`, run migrations sequentially (v1→v2, v2→v3, etc.).

---

### 3. Level Transition Integration

**Current state:** Save/load systems are independent of level transitions. There is no checkpoint-on-travel or restore-on-load integration.

**Additions needed:**

In `UAtlasLevelTransitionSubsystem::PreTravel`:
```cpp
// If request.bSaveCheckpointBeforeTravel:
SaveSystem->RequestSave("Checkpoint_Travel");
// Wait for save to complete before travel (use pre-travel ack pattern from 04_level_flow.md)
```

In `UAtlasLevelTransitionSubsystem::PostLoad`:
```cpp
// If the transition type implies loading state (e.g., returning to a saved game):
LoadSystem->RequestLoad(SavedSlotName);
```

---

### 4. Game Feature Integration

When a Game Feature plugin activates, it may add new savable actor types. The save collector will automatically pick up any actor with `UAtlasSavableComponent`, so **no changes are needed to the save pipeline for feature activation**.

However, actors added by features should be marked with a feature-specific tag so that save data can be cleanly removed when the feature is deactivated:

```cpp
// On UAtlasSavableComponent:
UPROPERTY(EditDefaultsOnly)
FGameplayTag OwningFeatureTag;  // e.g., Atlas.Feature.SampleCharacter
```

On feature deactivation: save data for actors with that tag can be optionally purged or ignored on next load.

---

### 5. Slot Manifest

**Current state:** No way to list available save slots without probing each slot name.

**Addition:**
```cpp
class GAMECORE_API FAtlasSaveSlotManifest
{
public:
    static bool AddSlot(const FString& SlotName, const FAtlasSaveGameMetadata& Metadata);
    static bool RemoveSlot(const FString& SlotName);
    static bool GetSlotMetadata(const FString& SlotName, FAtlasSaveGameMetadata& OutMetadata) const;
    static TArray<FAtlasSaveGameMetadata> GetAllSlots() const;

private:
    static FString GetManifestPath();
};
```

Manifest is stored as a small JSON or binary file at `Saved/SaveGames/SaveManifest.sav`.

---

### 6. Save System UDeveloperSettings

Add to `UAtlasDeveloperSettings`:
```cpp
UPROPERTY(EditDefaultsOnly, Category = "Save")
FString DefaultSaveSlotName = "PlayerSave_01";

UPROPERTY(EditDefaultsOnly, Category = "Save")
bool bEnableAutosave = true;

UPROPERTY(EditDefaultsOnly, Category = "Save")
float AutosaveIntervalSeconds = 300.0f;

UPROPERTY(EditDefaultsOnly, Category = "Save")
int32 AutosaveSlotCount = 5;

UPROPERTY(EditDefaultsOnly, Category = "Save")
bool bSaveOnLevelTransition = false;

UPROPERTY(EditDefaultsOnly, Category = "Save")
bool bLoadOnPostLevel = false;
```

---

## Architecture Diagram (Full Stack)

```
Game code / UI
    ↓ calls
IAtlasSaveSystem::RequestSave(SlotName)
    ↓
FAtlasSaveScheduler::EnqueueRequest(Manual)
    ↓ when queue is processed
FAtlasSaveSystem::StartSaveRequest(Request)
    ↓ game thread
FAtlasSaveCollector::CollectWorld(World)
    → walks actors with UAtlasSavableComponent
    → calls IAtlasSavable::CaptureState(SaveContext) on actor + components
    → produces FAtlasWorldSnapshot
    ↓
FAtlasBinaryWriter::Serialize(WorldSnapshot) [async]
    → writes versioned header + chunked actor data
    → produces TArray<uint8>
    ↓
FAtlasFileStorage::SaveToSlot(SlotName, Data) [async]
    → writes to Saved/SaveGames/{SlotName}.sav (via .tmp → rename)
    ↓ game thread callback
FAtlasSaveSystem::HandleSaveCompleted(Request, bSuccess)
    → logs result, fires delegate, processes next queue entry

Load (reverse):
IAtlasLoadSystem::RequestLoad(SlotName)
    ↓
FAtlasFileStorage::LoadFromSlot(SlotName) [async]
    ↓
FAtlasBinaryReader::Deserialize(Data) [async]
    → reads header, validates version, runs migration if needed
    → produces FAtlasWorldSnapshot
    ↓ game thread
FAtlasLoadSystem::ApplyWorldSnapshot(World, Snapshot)
    → for each FAtlasActorSnapshot:
        - find existing actor by FGuid (via UAtlasSavableComponent)
        - OR spawn actor from ClassPath
        - call IAtlasSavable::RestoreState(LoadContext) on actor + components
```

---

## Save Participant Contract

To participate in save/load, an actor must:

1. Have `UAtlasSavableComponent` attached (provides stable `FGuid ActorId`)
2. Implement `IAtlasSavable` on the actor itself and/or any of its components
3. Implement `CaptureState(FAtlasSaveContext&)` — serialize relevant data
4. Implement `RestoreState(const FAtlasLoadContext&)` — deserialize and apply

This is the only required contract. The save system handles everything else.

---

## Key Design Decisions and Trade-offs

| Decision | Trade-off |
|---|---|
| Binary serialization (FArchive) vs JSON | Binary: smaller, faster, no schema. JSON: human-readable, easier debugging. **Binary chosen** — appropriate for games. Add a debug dump to JSON for tooling. |
| Flat FAtlasSaveGameSnapshot vs FAtlasWorldSnapshot | Flat store: good for global game state (flags, player progress). World snapshot: good for per-actor state. Both exist and serve different purposes. |
| Two-phase async (serialize then write) | Allows cancellation between phases, keeps serialization deterministic while I/O is offloaded. |
| Autosave ring buffer of 5 slots | Balances disk space, protects against corruption in any one slot. Index persisted separately. |
| Manual save > Event save > Autosave priority | Ensures explicit user intent is never blocked by background saves. |
