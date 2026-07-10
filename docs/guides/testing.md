# Testing

How to validate each Atlas system. There are no automated tests yet; when added they should use Unreal's Automation framework (`FAutomationTestBase`) under `Source/<Module>/Private/Tests/`.

## Play-In-Editor basics

- PIE exercises most systems, but note: the startup flow's auto-travel to the main menu is **deliberately skipped in PIE** — test it in Standalone Game mode or a packaged build.
- Viewport widget re-adding after travel behaves differently in PIE vs standalone; verify loading screens in standalone at least once.

## Core runtime

1. PIE and check the Output Log for `[Core]` lines: systems registry, save/load system init, subsystem lifecycles.
2. `UAtlasAssetManager` logs `Asset manager initialized` at startup — if missing, `AssetManagerClassName` in `DefaultEngine.ini` is wrong.
3. Project Settings → Game should show **Atlas Framework** and **Atlas UI** sections.

## UI

1. Configure a screen registry with a test screen ([common_ui.md](common_ui.md)).
2. In PIE, call `PushScreen` from Blueprint or a cheat: verify the widget appears on its layer, input mode switches, and Escape/gamepad-B pops it.
3. Verify `ShowNotification` toasts auto-dismiss and `ShowModal` fires the result delegate for both buttons.

## Level flow

1. Standalone: boot → verify auto-travel to the main menu map and `Game flow state` log transitions.
2. Call `RequestTransition` to a gameplay map: loading screen shows, `OnPreTravel` then `OnPostLoad` fire (add temporary logs), loading screen hides.
3. Confirm the map's `AAtlasPostLoadTrigger` fires (`Transition post-load` log) and back-to-menu works.

## GAS

1. Author the pawn data chain ([gas.md](gas.md)) and possess the pawn in PIE.
2. `showdebug abilitysystem`: verify owner = PlayerState, avatar = pawn, ability sets granted, attributes attached.
3. Spawn an AI pawn with an `AAtlasAIController`: verify it gets its own ASC.
4. Destroy and respawn the player pawn: PlayerState ASC persists; re-possession re-runs `InitAbilityActorInfo` with the new avatar.
5. Multiplayer: PIE with 2 players (listen server) — verify client-side init lands via `OnRep_PlayerState`.

## Input

1. Press a bound key: verify `AbilityInputTagPressed` reaches the ASC (temporary log) and the ability activates on `Started`, releases on `Completed`.
2. Test a gamepad if available; verify the glyph subsystem broadcasts the device change.

## Game Features

1. Activate a sample feature from the Game Features editor panel.
2. Verify grants/screens appear; spawn a pawn *after* activation and verify it is covered (extension handler path).
3. Deactivate: verify abilities revoke, screens pop and unregister, HUD elements disappear.

## Save/load

1. PIE: trigger a save (`RequestSave`) and inspect `Saved/SaveGames/` — slot `.sav` plus `SaveManifest.json` listing it.
2. Move a savable actor, save, move it again, load — verify the transform restores.
3. Set `bSaveCheckpointBeforeTravel` on a transition — verify `Checkpoint_Travel.sav` is written before the new map loads.
4. Hex-edit the version in a save header — verify the load fails with a missing-migration error rather than crashing; register a dummy migration and verify the `Save data migrated` log.
5. Verify autosave slots rotate through `Autosave_0..4`.
