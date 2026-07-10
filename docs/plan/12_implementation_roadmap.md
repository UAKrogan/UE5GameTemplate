# 12 — Implementation Roadmap

Each phase is safe and independently reviewable. No phase depends on an incomplete previous phase — each leaves the project in a compilable, testable state.

---

## Phase 0 — Repository Audit and Baseline Cleanup

**Goal:** Clean up the existing code to remove risks identified in the analysis before adding anything new.

**Files to change:**
- `Source/GameCore/Private/Systems/Save/AtlasSaveSystem.cpp` — change `bSaveInProgress` to `TAtomic<bool>` or add comment documenting game-thread-only constraint
- `Source/GameCore/Private/Systems/Load/AtlasLoadSystem.cpp` — same for `bLoadInProgress`
- `Source/GameCore/Public/Subsystems/AtlasGameInstanceSubsystem.h` — document `OwningSubsystem` raw pointer lifetime constraint
- `Source/GameActors/Public/GameMode/AtlasGameMode.h` — evaluate `AGameModeBase` vs `AGameMode` (document decision, or change base class)
- `Config/DefaultEngine.ini` — remove duplicate `DefaultGraphicsRHI` line
- `Config/DefaultEngine.ini` — remove Android File Server section (not relevant to desktop template)

**New files:**
- `docs/guides/conventions.md` — write coding conventions guide

**Module dependency changes:** None.

**Risks:** Low — cleanup only.

**How to test:** Build and verify no compile errors. Open editor, PIE should work as before.

**Expected result:** Clean baseline. All existing warnings/risks documented or resolved.

---

## Phase 1 — Module Boundaries and Dependency Finalization

**Goal:** Lock in the final module dependency graph. Add `GameplayAbilities` to `GameActors.Build.cs`. Enable `GameFeatures` and `ModularGameplay` plugins.

**Files to change:**
- `Source/GameActors/GameActors.Build.cs` — add `GameplayAbilities`, `GameplayTags`, `GameplayTasks` to public deps
- `Game.uproject` — add `GameFeatures` and `ModularGameplay` plugin entries (enabled)
- `Game.uproject` — add `GameplayAbilities` plugin entry (enabled)

**New files:**
- `docs/guides/architecture.md`
- `docs/guides/modules.md`

**Module dependency changes:** `GameActors` gains GAS engine module deps.

**Risks:** Medium — enabling `GameFeatures` and `ModularGameplay` plugins changes how the engine initializes. Test PIE thoroughly after.

**How to test:** Build (Editor). PIE. Verify no crashes. Check Output Log for plugin initialization messages.

**Expected result:** Project compiles with all required engine module deps. Feature plugin infrastructure is available.

---

## Phase 2 — Project Settings, Asset Manager, Gameplay Tags

**Goal:** Add `UAtlasDeveloperSettings`, `UAtlasAssetManager`, and gameplay tag initialization.

**New classes:**
- `GameCore/Public/Settings/AtlasDeveloperSettings.h` + `.cpp`
- `GameCore/Public/AssetManager/AtlasAssetManager.h` + `.cpp`
- `GameCore/Public/Tags/AtlasGameplayTags.h` + `.cpp` (native tag declarations)

**Files to change:**
- `Config/DefaultEngine.ini` — add `AssetManagerClassName=/Script/GameCore.AtlasAssetManager`
- `Config/DefaultGame.ini` — add `[/Script/GameCore.AtlasDeveloperSettings]` section with default map paths

**Module dependency changes:** None (GameCore already has required engine deps).

**Risks:** Low — new classes, no changes to existing code.

**How to test:** Open Project Settings in editor. Verify `Atlas Framework` category appears. Set `StartupMap`, `MainMenuMap`. Verify asset manager logs its init message.

**Expected result:** Framework settings are configurable from the editor. Asset manager initialized. Native tags registered.

---

## Phase 3 — Common UI Foundation

**Goal:** Build the layered UI subsystem, base widget classes, screen registry, and input mode management.

**New classes:**
- `GameUI/Public/Subsystems/AtlasUISubsystem.h` + `.cpp` (extend existing stub — full implementation)
- `GameUI/Public/Widgets/AtlasRootWidget.h` + `.cpp`
- `GameUI/Public/Widgets/AtlasActivatableWidget.h` + `.cpp`
- `GameUI/Public/Widgets/AtlasHUDWidget.h` + `.cpp`
- `GameUI/Public/Widgets/AtlasMenuWidget.h` + `.cpp`
- `GameUI/Public/Widgets/AtlasModalWidget.h` + `.cpp`
- `GameUI/Public/Widgets/AtlasNotificationWidget.h` + `.cpp`
- `GameUI/Public/Widgets/AtlasLoadingScreenWidget.h` + `.cpp`
- `GameUI/Public/Data/AtlasScreenDefinition.h` + `.cpp`
- `GameUI/Public/Data/AtlasScreenRegistry.h` + `.cpp`
- `GameUI/Public/Data/AtlasInputGlyphData.h` + `.cpp`
- `GameUI/Public/Subsystems/AtlasGlyphSubsystem.h` + `.cpp`

**Files to change:**
- `GameUI/GameUI.Build.cs` — verify `CommonUI` dep is present (already is)

**Module dependency changes:** None.

**New Blueprint assets (in editor):**
- `Content/UI/WBP_AtlasRoot.uasset` (root widget Blueprint)
- `Content/UI/WBP_AtlasLoadingScreen.uasset`
- `Content/UI/DA_AtlasScreenRegistry.uasset`

**Risks:** Medium — Common UI has specific initialization requirements. The root widget must be added to viewport correctly. Input mode switching must be validated with both keyboard and gamepad in PIE.

**How to test:**
1. PIE
2. Call `UAtlasUISubsystem::PushScreen("TestScreen")` from Blueprint or console
3. Verify widget appears on correct layer
4. Verify input mode changes
5. Verify back navigation pops the screen

**Expected result:** Functional layered UI system. Screens can be pushed and popped by ID. Loading screen can be shown and hidden.

---

## Phase 4 — Level Transition and Loading Screen

**Goal:** Implement the game flow state machine and level transition lifecycle.

**New classes:**
- `GameCore/Public/Subsystems/AtlasGameFlowSubsystem.h` + `.cpp`
- `GameCore/Public/Subsystems/AtlasLevelTransitionSubsystem.h` + `.cpp`
- `GameCore/Public/Subsystems/AtlasLoadingScreenSubsystem.h` + `.cpp`
- `GameActors/Public/Utility/AtlasPostLoadTrigger.h` + `.cpp`

**New content:**
- `Content/Maps/MainMenu.umap` — minimal main menu map
- `Content/Maps/Startup.umap` — minimal startup map (optional, can skip to main menu)
- Place `AAtlasPostLoadTrigger` in each map

**Files to change:**
- `Config/DefaultGame.ini` — set map paths in `UAtlasDeveloperSettings`
- `Config/DefaultEngine.ini` — set `GameDefaultMap` to startup or main menu map
- `Game/Private/Runtime/AtlasGameInstance.cpp` — trigger startup flow after `Init()`

**Module dependency changes:** None.

**Risks:** Medium — level travel in PIE behaves differently from standalone. `NotifyLevelLoaded` timing must be validated. Loading screen must remain visible across level travel.

**How to test:**
1. PIE from startup map → observe transition to main menu
2. Call `UAtlasLevelTransitionSubsystem::RequestTransition()` from Blueprint → observe loading screen, travel, post-load
3. Verify `OnPreTravel` and `OnPostLoad` delegates fire
4. Test return to main menu

**Expected result:** Complete level flow works. Loading screen visible during travel. Pre/post hooks fire in correct order.

---

## Phase 5 — GAS-Ready Actor Foundation

**Goal:** Add GAS initialization to all actor classes via the extension component pattern. Implement `UAtlasPawnExtensionComponent`, `UAtlasAbilityExtensionComponent`, base ASC, and base attribute set.

**New classes:**
- `GameActors/Public/Components/AtlasPawnExtensionComponent.h` + `.cpp`
- `GameActors/Public/Components/AtlasAbilityExtensionComponent.h` + `.cpp`
- `GameActors/Public/Components/AtlasMovementExtensionComponent.h` + `.cpp`
- `GameActors/Public/GAS/AtlasAbilitySystemComponent.h` + `.cpp`
- `GameActors/Public/GAS/AtlasBaseAttributeSet.h` + `.cpp`
- `GameActors/Public/GAS/AtlasBaseGameplayAbility.h` + `.cpp`
- `GameActors/Public/Data/AtlasAbilitySet.h` + `.cpp`
- `GameActors/Public/Data/AtlasPawnData.h` + `.cpp`

**Files to change:**
- `GameActors/Public/Character/AtlasCharacter.h` + `.cpp` — add `PawnExtComp`, GAS interface, possession hooks
- `GameActors/Public/Pawn/AtlasPawn.h` + `.cpp` — add `PawnExtComp`, GAS interface, possession hooks
- `GameActors/Public/PlayerState/AtlasPlayerState.h` + `.cpp` — add ASC, implement `IAbilitySystemInterface`
- `GameActors/Public/Controller/AtlasPlayerController.h` + `.cpp` — `OnPossess`/`OnUnPossess` hooks
- `GameActors/Public/Controller/AtlasAIController.h` + `.cpp` — `OnPossess`/`OnUnPossess` with AI ASC init

**Module dependency changes:**
- `GameActors.Build.cs` — already done in Phase 1

**Risks:** High — GAS initialization order is subtle, especially with replication. PlayerState-owned ASC must be initialized on both server and client. `OnRep_PlayerState` must trigger `InitializeAbilitySystem`.

**How to test:**
1. PIE with two clients (if multiplayer) or single player
2. Use Gameplay Debugger (backtick key → `showdebug abilitysystem`) to verify ASC is initialized
3. Verify attribute sets are attached
4. Spawn an AI pawn, verify its own ASC initializes correctly
5. Kill pawn and respawn — verify PlayerState ASC persists

**Expected result:** All Atlas actor types are GAS-ready. ASC initializes correctly for both player-controlled and AI-controlled pawns.

---

## Phase 6 — Enhanced Input and Ability Input Binding

**Goal:** Wire Enhanced Input to ability activation via gameplay tags. Implement `UAtlasInputExtensionComponent` and input config data assets.

**New classes:**
- `GameActors/Public/Components/AtlasInputExtensionComponent.h` + `.cpp`
- `GameActors/Public/Data/AtlasInputConfigData.h` + `.cpp`

**New content:**
- `Content/Input/IA_Atlas_Jump.uasset`
- `Content/Input/IMC_Atlas_Default.uasset`
- `Content/Input/DA_Atlas_InputConfig.uasset`

**Files to change:**
- `GameActors/Public/Controller/AtlasPlayerController.h` + `.cpp` — add `InputExtComp`, wire `SetupInputComponent`
- `GameActors/Public/Character/AtlasCharacter.h` + `.cpp` — forward input setup to `InputExtComp` on `SetupPlayerInputComponent`
- `GameActors/Public/GAS/AtlasAbilitySystemComponent.h` + `.cpp` — add `AbilityInputTagPressed` / `AbilityInputTagReleased`

**Module dependency changes:** None (EnhancedInput already in GameActors.Build.cs).

**Risks:** Medium — Enhanced Input binding order relative to GAS initialization must be correct. Input must not be bound before ASC has abilities granted.

**How to test:**
1. PIE
2. Press the jump input action key
3. Verify `AbilityInputTagPressed(Atlas.Input.Ability.Jump)` is called (add log)
4. Verify ability activates
5. Test gamepad binding (if gamepad available)

**Expected result:** Input actions correctly activate GAS abilities via gameplay tags.

---

## Phase 7 — Game Features and Modular Gameplay Support

**Goal:** Add `UGameFrameworkComponentManager` receiver calls to all base actors. Implement custom Game Feature actions.

**New classes:**
- `GameActors/Public/Actions/AtlasGameFeatureAction_AddAbilities.h` + `.cpp`
- `GameActors/Public/Actions/AtlasGameFeatureAction_AddInputConfig.h` + `.cpp`
- `GameUI/Public/Actions/AtlasGameFeatureAction_AddScreens.h` + `.cpp`
- `GameUI/Public/Actions/AtlasGameFeatureAction_AddHUDElements.h` + `.cpp`

**Files to change:**
- All base actor `BeginPlay()` and `EndPlay()` implementations — add `AddGameFrameworkComponentReceiver` / `RemoveGameFrameworkComponentReceiver`
- `GameCore/Public/Settings/AtlasDeveloperSettings.h` + `.cpp` — add `bAutoActivateGameFeatures` handling
- `GameCore/Private/Systems/AtlasSystemsRegistry.cpp` — add feature policy initialization

**Module dependency changes:**
- `GameActors.Build.cs` — add `ModularGameplay` to deps
- `GameCore.Build.cs` — add `GameFeatures` to deps

**Risks:** Medium — `UGameFrameworkComponentManager` must be initialized before actors spawn. Timing with subsystem initialization order matters.

**How to test:**
1. Create a minimal test Game Feature plugin
2. Activate it via `UGameFeaturesSubsystem`
3. Verify `UGameFeatureAction_AddComponents` adds a component to `AAtlasCharacter`
4. Deactivate the feature and verify the component is removed
5. Test activation with actors already in the world

**Expected result:** Base actors participate in Modular Gameplay receiver pattern. Custom actions can add abilities and UI screens from feature plugins.

---

## Phase 8 — Sample Game Feature Plugins

**Goal:** Create sample feature plugins demonstrating the full pipeline.

**New plugins:**
- `Plugins/GameFeatures/GameFeature_SampleCharacter/`
  - `GFD_SampleCharacter.uasset`
  - `GA_Sample_Jump.uasset`
  - `DA_Sample_AbilitySet.uasset`
  - `DA_Sample_PawnData.uasset`
  - `IMC_Sample_Character.uasset`
- `Plugins/GameFeatures/GameFeature_SampleUI/`
  - `GFD_SampleUI.uasset`
  - `WBP_Sample_MainMenu.uasset`
  - `DA_Sample_ScreenDefinition.uasset`

**New source files:**
- None required if using only data-driven actions. Add source only if demonstrating a custom ability with C++ logic.

**Risks:** Low — feature plugins are additive, no base module changes.

**How to test:**
1. Activate `GameFeature_SampleCharacter`
2. PIE — verify sample abilities are granted to the player character
3. Activate `GameFeature_SampleUI`
4. PIE — verify sample main menu screen is registered and functional
5. Deactivate both features — verify everything cleans up

**Expected result:** Functional sample feature plugins demonstrating every feature action type.

---

## Phase 9 — Save/Load Foundation Hardening

**Goal:** Fix threading safety, add versioning migration, add slot manifest, integrate with level transition.

**Files to change:**
- `GameCore/Private/Systems/Save/AtlasSaveSystem.cpp` — use `TAtomic<bool>` for `bSaveInProgress`
- `GameCore/Private/Systems/Load/AtlasLoadSystem.cpp` — same for `bLoadInProgress`
- `GameCore/Private/Systems/Serialization/AtlasBinaryReader.cpp` — add migration path invocation
- `GameCore/Public/Subsystems/AtlasLevelTransitionSubsystem.h` + `.cpp` — add pre-travel save checkpoint + post-load restore hooks

**New classes:**
- `GameCore/Public/Systems/Save/AtlasSaveMigrationManager.h` + `.cpp`
- `GameCore/Public/Systems/Save/AtlasSaveSlotManifest.h` + `.cpp`

**Module dependency changes:** None.

**Risks:** Low — targeted additions to existing code.

**How to test:**
1. PIE — save a game
2. Inspect `Saved/SaveGames/` for correct file output
3. Load the save — verify actor state restored
4. Trigger level transition with `bSaveCheckpointBeforeTravel = true` — verify save completes before travel
5. Corrupt the version number in the save file — verify migration log appears
6. Query slot manifest — verify all slots listed

**Expected result:** Save/load pipeline is thread-safe, version-aware, and integrated with level transitions.

---

## Phase 10 — Documentation and Template Polish

**Goal:** Write all user-facing guides. Create placeholder map assets. Finalize CLAUDE.md and README.

**New files:**
- `docs/guides/README.md`
- `docs/guides/architecture.md`
- `docs/guides/modules.md`
- `docs/guides/setup.md`
- `docs/guides/common_ui.md`
- `docs/guides/level_flow.md`
- `docs/guides/gas.md`
- `docs/guides/game_features.md`
- `docs/guides/actors.md`
- `docs/guides/conventions.md`
- `docs/guides/testing.md`
- Update `CLAUDE.md` with final state
- Update root `README.md` to reflect final state

**New content:**
- `Content/Maps/MainMenu.umap` — if not already created
- `Content/Maps/Startup.umap` — if not already created
- Verify all `UAtlasDeveloperSettings` defaults point to these maps

**Risks:** None — documentation only, plus content map creation.

**How to test:**
1. Follow `docs/guides/setup.md` as if you were a new developer — verify the guide is accurate
2. Verify editor opens cleanly with no missing asset warnings
3. PIE end-to-end: startup → menu → gameplay → save → return to menu → load

**Expected result:** Complete, documented, polished template ready for use as a base for game projects.

---

## Phase Summary Table

| Phase | Risk | Compile Changes | Content Changes | New Classes |
|---|---|---|---|---|
| 0 | Low | Minor (atomics) | No | No |
| 1 | Medium | .uproject, Build.cs | No | No |
| 2 | Low | GameCore | No | 3 |
| 3 | Medium | GameUI | Yes (widgets) | ~12 |
| 4 | Medium | GameCore, GameActors | Yes (maps) | 4 |
| 5 | High | GameActors | No | ~9 |
| 6 | Medium | GameActors | Yes (input assets) | 2 |
| 7 | Medium | GameActors, GameCore | No | 4 |
| 8 | Low | None (data-driven) | Yes (plugins) | 0–2 |
| 9 | Low | GameCore | No | 2 |
| 10 | None | No | Yes (maps, docs) | 0 |
