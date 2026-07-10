# 18 — Genre Support Roadmap (Phases 12–18)

Continues the completed roadmap (phases 0–11). Same rules: each phase leaves the project compilable, is committed separately, and keeps the module dependency graph inviolate. Design sources: `14_genre_gap_analysis.md`, `15_camera_modes.md`, `16_interaction_teams.md`, `17_selection_orders_rts.md`.

---

## Phase 12 — Framework Remnants & Quality Seams

**Goal:** clear the tracked debt genre work will lean on (from `docs/plan/README.md` + save/load caveats).

**Changes:**
- Save/load completion delegates (`OnSaveCompleted/OnLoadCompleted` on the systems) + Blueprint facade: `UAtlasPersistenceSubsystem` (GameCore, `UGameInstanceSubsystem`) wrapping save/load/autosave/slot-manifest with UFUNCTIONs and BlueprintAssignable events.
- Stable component IDs for save chunks: persistent per-component `FGuid` on `UAtlasSavableComponent`-registered components (replaces index+name chunk keys; keep a legacy-name fallback read path, bump binary version, register a v1→v2 migration).
- Pre-travel acknowledgements: `RegisterPreTravelAck/Handle` on `UAtlasLevelTransitionSubsystem` with timeout (default 5 s); save system holds travel until an in-flight save completes.
- `Atlas: Add Pawn Data` feature action; `UAtlasSavableComponent::OwningFeatureTag`.
- First automation tests (`Source/GameCore/Private/Tests/`): scheduler priority/debounce, binary round-trip + migration, file storage failure cases, slot manifest.

**Risks:** Medium — binary format version bump; the migration path gets its first real use.
**Test:** automation tests green; save in old format loads via migration; travel waits for a slow fake ack until timeout.

---

## Phase 13 — Camera Mode Framework

**Goal:** implement `15_camera_modes.md`.

**New classes (GameActors):** `UAtlasCameraMode` (+`FAtlasCameraView`), `UAtlasCameraMode_Config`, `_FirstPerson`, `_ThirdPerson`, `_TopDown`, `UAtlasCameraModeAbility`.
**Changed:** `UAtlasCameraExtensionComponent` (mode stack + blending; Push/ClearCameraConfig become stack sugar), `UAtlasPawnData` (`DefaultCameraMode`, per-mode input config map), input extension (apply per-mode configs on `OnCameraModeChanged`), GameCore tags (`Atlas.Camera.Mode.*`).
**Risks:** Medium — blend math vs spring-arm collision; tick management on the component.
**Test:** FP↔TP toggle ability blends smoothly; vehicle enter/exit still overrides; top-down mode works on a plain pawn.

---

## Phase 14 — Interaction & Teams

**Goal:** implement `16_interaction_teams.md`.

**New classes (GameActors):** `IAtlasInteractable` + `UAtlasInteractableComponent`, `UAtlasInteractionComponent` (LineTrace/Overlap/CursorTrace detection, timer-driven), `UAtlasInteractAbility`; `Teams/`: `EAtlasTeamAttitude`, `UAtlasTeamConfig`, `UAtlasTeamComponent`, `UAtlasTeamStatics`.
**Changed:** `AAtlasPlayerState`/`AAtlasAIController` implement `IGenericTeamAgentInterface`; `UAtlasDeveloperSettings` gains `TeamConfig` soft path; GameCore tags (`Atlas.Input.Ability.Interact`, `Atlas.Interaction.Blocked`).
**Risks:** Low–Medium — trace channel/config choices; authority path for `Interact`.
**Test:** focus/prompt/interact loop in TP and cursor modes; attitude matrix drives `GetAttitude` and AI perception affiliation.

---

## Phase 15 — Selection & Orders

**Goal:** implement the genre-agnostic half of `17_selection_orders_rts.md`.

**New classes (GameActors):** `UAtlasSelectableComponent`, `UAtlasSelectionSubsystem` (ULocalPlayerSubsystem; single/multi/rect select, control groups), `FAtlasOrder`, `UAtlasOrderComponent` (gameplay-event routing + non-ASC delegate fallback + optional queue), `UAtlasOrderStatics`.
**Changed:** GameCore tags (`Atlas.Order.*`, `Atlas.Input.Native.Select/Command/Camera*`).
**Risks:** Medium — screen-rect selection correctness; order validation/authority seam.
**Test:** select own units only; control groups; order event reaches an event-triggered ability; RPG-style single-select + companion order works without any RTS class.

---

## Phase 16 — RTS Player Shell

**Goal:** the RTS-facing classes from `17_selection_orders_rts.md`.

**New classes (GameActors):** `AAtlasStrategyPawn` (pan/zoom/edge-scroll, top-down camera mode, native camera input), `AAtlasStrategyPlayerController` (cursor gameplay, LMB select/marquee, RMB context orders via virtual `ResolveContextOrder`, `ServerIssueOrder` RPC, shift-queue).
**Risks:** Medium — input routing between native camera controls, selection clicks, and UI under `GameAndMenu` mode.
**Test:** full RTS loop in a blockout map: move camera, select, marquee, move/attack/stop orders, control groups, shift-queue.

---

## Phase 17 — Sample Genre Feature Plugins

**Goal:** deletable demos proving both genres end-to-end (content-only where possible; minimal C++ only if an ability needs it).

**New plugins:**
- `Plugins/GameFeatures/GameFeature_SampleRPG`: FP/TP character pawn data (default TP mode + toggle-camera ability + interact ability), one interactable pickup demo, inventory demo via existing `UAtlasInventorySaveAdapter`, HUD prompt widget injected through `Atlas: Add HUD Elements`.
- `Plugins/GameFeatures/GameFeature_SampleRTS`: strategy pawn/controller pawn data, three selectable demo units with event-triggered Move/Stop abilities, team setup (player vs hostile dummy), minimal command HUD.

CLI deliverable: `.uplugin` + READMEs listing exact editor assets (same pattern as Phase 8); editor session authors the assets.
**Risks:** Low — additive.
**Test:** activate RPG feature → walk/interact/toggle camera; activate RTS feature → select/command loop; deactivate both → clean.

---

## Phase 18 — Documentation & Validation

**Goal:** guides for every new system + full manual validation pass.

- New guides: `camera_modes.md`, `interaction_teams.md`, `selection_orders.md`, `building_an_rts.md`, `building_an_rpg.md` (genre how-tos walking through the sample plugins).
- Update: `actors.md`, `modules.md`, `gas.md` (event-triggered order abilities), `testing.md`, `CLAUDE.md`, `README.md`, `PROJECT_AI_CONTEXT.txt`.
- Manual PIE/standalone validation per `testing.md` including the new sections; fix what falls out.

---

## Phase Summary

| Phase | Scope | Risk | Modules touched |
|---|---|---|---|
| 12 | Remnants: delegates, BP facade, component IDs, acks, tests | Medium | GameCore |
| 13 | Camera mode stack (FP/TP/top-down) | Medium | GameActors, GameCore (tags) |
| 14 | Interaction + teams | Low–Med | GameActors, GameCore (tags/settings) |
| 15 | Selection + orders | Medium | GameActors, GameCore (tags) |
| 16 | RTS strategy pawn/controller | Medium | GameActors |
| 17 | Sample RPG/RTS feature plugins | Low | Plugins only |
| 18 | Docs + validation | None | docs |

Ordering rationale: 12 hardens seams everything else reuses; 13 unblocks both genres' cameras; 14–15 are independent of each other but both precede 16; 17–18 close the loop. RPG-only projects can stop after 14 (+13); RTS-only projects need 13–16.
