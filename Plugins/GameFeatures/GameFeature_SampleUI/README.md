# GameFeature_SampleUI

`[TEMPLATE EXAMPLE]` — demonstrates contributing UI screens from a Game Feature plugin. Content-only (no C++). Safe to delete in real projects.

## Assets to author in the editor (`Content/`)

| Asset | Type | Purpose |
|---|---|---|
| `GFD_SampleUI` | `UGameFeatureData` | Feature descriptor; lists the actions below |
| `WBP_Sample_MainMenu` | Widget Blueprint of `UAtlasMenuWidget` | Sample main menu screen |
| `DA_Sample_ScreenDefinition` | `UAtlasScreenDefinition` | ScreenId `SampleMainMenu`, widget `WBP_Sample_MainMenu`, layer `Menu` |

## Actions in `GFD_SampleUI`

- `Atlas: Add Screens` — Screens: `SampleMainMenu` → `DA_Sample_ScreenDefinition`

## How to test

1. Activate the feature.
2. In PIE, run `UAtlasUISubsystem::PushScreen("SampleMainMenu")` from Blueprint or a cheat.
3. Verify the menu appears on the Menu layer and input mode switches to Menu.
4. Deactivate the feature — verify the screen is popped and unregistered.
