# 10 — Settings and Data Assets

---

## When to Use Project Settings vs. Data Assets

| Criteria | Use `UDeveloperSettings` | Use Data Asset |
|---|---|---|
| Per-project, single instance | Yes | No |
| Appears in Project Settings panel | Yes | No |
| Multiple instances per project | No | Yes |
| Blueprint/data-driven authoring | Partial | Yes |
| Can be swapped per game mode or level | No | Yes |
| Can be async-loaded | No (always loaded) | Yes |
| Needs to be referenced from multiple places | Better as data asset | Yes |

**Rule:** Use `UDeveloperSettings` for framework-wide configuration that is truly singular (one value per project). Use data assets for configuration that is authored per-pawn-type, per-screen, per-feature, or in multiples.

---

## `UAtlasDeveloperSettings`

**Module:** `GameCore`
**Extends:** `UDeveloperSettings`
**Category:** `"Atlas Framework"` (appears under Project Settings → Game → Atlas Framework)
**Blueprint-friendly:** Readable only

```cpp
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Atlas Framework"))
class GAMECORE_API UAtlasDeveloperSettings : public UDeveloperSettings
{
    // ── Maps ────────────────────────────────────────────────────
    UPROPERTY(Config, EditDefaultsOnly, Category="Maps")
    TSoftObjectPtr<UWorld> StartupMap;

    UPROPERTY(Config, EditDefaultsOnly, Category="Maps")
    TSoftObjectPtr<UWorld> MainMenuMap;

    UPROPERTY(Config, EditDefaultsOnly, Category="Maps")
    TSoftObjectPtr<UWorld> DefaultGameplayMap;

    // ── Actors ──────────────────────────────────────────────────
    UPROPERTY(Config, EditDefaultsOnly, Category="Actors")
    TSoftObjectPtr<UAtlasPawnData> DefaultPawnData;

    // ── UI ──────────────────────────────────────────────────────
    UPROPERTY(Config, EditDefaultsOnly, Category="UI")
    TSoftClassPtr<UAtlasLoadingScreenWidget> LoadingScreenWidgetClass;

    UPROPERTY(Config, EditDefaultsOnly, Category="UI")
    TSoftObjectPtr<UAtlasScreenRegistry> ScreenRegistry;

    // ── Loading ─────────────────────────────────────────────────
    UPROPERTY(Config, EditDefaultsOnly, Category="Loading")
    float LoadingScreenFadeInDuration = 0.25f;

    UPROPERTY(Config, EditDefaultsOnly, Category="Loading")
    float LoadingScreenFadeOutDuration = 0.25f;

    // ── Save ────────────────────────────────────────────────────
    UPROPERTY(Config, EditDefaultsOnly, Category="Save")
    FString DefaultSaveSlotName = TEXT("PlayerSave_01");

    UPROPERTY(Config, EditDefaultsOnly, Category="Save")
    bool bEnableAutosave = true;

    UPROPERTY(Config, EditDefaultsOnly, Category="Save")
    float AutosaveIntervalSeconds = 300.0f;

    UPROPERTY(Config, EditDefaultsOnly, Category="Save")
    bool bSaveCheckpointOnTravel = false;

    // ── Features ────────────────────────────────────────────────
    UPROPERTY(Config, EditDefaultsOnly, Category="Features")
    bool bAutoActivateGameFeatures = true;

    // ── Asset Manager ───────────────────────────────────────────
    UPROPERTY(Config, EditDefaultsOnly, Category="AssetManager")
    TArray<FPrimaryAssetTypeInfo> AdditionalPrimaryAssetTypes;

    // Static accessor
    static const UAtlasDeveloperSettings* Get();
};
```

Config storage: values in `Config/DefaultGame.ini` under `[/Script/GameCore.AtlasDeveloperSettings]`.

---

## `UAtlasUIDeveloperSettings`

**Module:** `GameUI`
**Extends:** `UDeveloperSettings`
**Category:** `"Atlas UI"`
**Blueprint-friendly:** Readable

```cpp
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Atlas UI"))
class GAMEUI_API UAtlasUIDeveloperSettings : public UDeveloperSettings
{
    UPROPERTY(Config, EditDefaultsOnly, Category="Input Glyphs")
    TSoftObjectPtr<UAtlasInputGlyphData> InputGlyphData;

    UPROPERTY(Config, EditDefaultsOnly, Category="Input Glyphs")
    bool bAutoDetectInputDevice = true;

    UPROPERTY(Config, EditDefaultsOnly, Category="Navigation")
    bool bEnableGamepadNavigation = true;

    UPROPERTY(Config, EditDefaultsOnly, Category="Navigation")
    float AnalogNavigationRepeatDelay = 0.5f;
};
```

---

## Data Assets

### `UAtlasPawnData`

**Module:** `GameActors`  
**Extends:** `UPrimaryDataAsset`  
**Purpose:** Per-pawn-type blueprint for GAS, input, and camera setup.  
**Primary Asset Type:** `AtlasPawnData`

```cpp
UCLASS(BlueprintType)
class GAMEACTORS_API UAtlasPawnData : public UPrimaryDataAsset
{
    UPROPERTY(EditDefaultsOnly, Category="Pawn")
    TSoftClassPtr<APawn> PawnClass;

    UPROPERTY(EditDefaultsOnly, Category="Abilities")
    TArray<TSoftObjectPtr<UAtlasAbilitySet>> AbilitySets;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    TArray<FAtlasInputConfig> InputConfigs;

    UPROPERTY(EditDefaultsOnly, Category="Camera")
    TSoftObjectPtr<UAtlasCameraConfig> CameraConfig;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
```

---

### `UAtlasAbilitySet`

**Module:** `GameActors`  
**Extends:** `UPrimaryDataAsset`  
**Purpose:** Bundle of abilities, attribute sets, and startup effects.  
**Primary Asset Type:** `AtlasAbilitySet`

See `07_gas_architecture.md` for full definition.

---

### `UAtlasInputConfig`

**Module:** `GameActors`  
**Extends:** `UDataAsset`  
**Purpose:** Maps an `UInputMappingContext` with a priority and a set of ability input bindings.  

```cpp
UCLASS(BlueprintType)
class GAMEACTORS_API UAtlasInputConfigData : public UDataAsset
{
    UPROPERTY(EditDefaultsOnly)
    TSoftObjectPtr<UInputMappingContext> MappingContext;

    UPROPERTY(EditDefaultsOnly)
    int32 Priority = 0;

    UPROPERTY(EditDefaultsOnly)
    TArray<FAtlasAbilityInputBinding> AbilityBindings;
};
```

---

### `UAtlasScreenDefinition`

**Module:** `GameUI`  
**Extends:** `UPrimaryDataAsset`  
**Purpose:** Defines a single UI screen.  
**Primary Asset Type:** `AtlasScreenDefinition`

See `03_ui_architecture.md` for full definition.

---

### `UAtlasScreenRegistry`

**Module:** `GameUI`  
**Extends:** `UDataAsset`  
**Purpose:** Single registry data asset mapping screen IDs to definitions.

```cpp
UCLASS(BlueprintType)
class GAMEUI_API UAtlasScreenRegistry : public UDataAsset
{
    UPROPERTY(EditDefaultsOnly)
    TMap<FName, TSoftObjectPtr<UAtlasScreenDefinition>> Screens;

    const UAtlasScreenDefinition* FindScreen(FName ScreenId) const;
};
```

---

### `UAtlasCameraConfig`

**Module:** `GameActors`  
**Extends:** `UDataAsset`  
**Purpose:** Camera and spring arm configuration for a pawn type or movement mode.

```cpp
UCLASS(BlueprintType)
class GAMEACTORS_API UAtlasCameraConfig : public UDataAsset
{
    UPROPERTY(EditDefaultsOnly)
    float SpringArmLength = 400.0f;

    UPROPERTY(EditDefaultsOnly)
    FRotator RelativeRotation = FRotator(-15.f, 0.f, 0.f);

    UPROPERTY(EditDefaultsOnly)
    bool bUsePawnControlRotation = true;

    UPROPERTY(EditDefaultsOnly)
    float FieldOfView = 90.0f;

    UPROPERTY(EditDefaultsOnly)
    float BlendTime = 0.3f;
};
```

---

### `UAtlasInputGlyphData`

**Module:** `GameUI`  
**Extends:** `UDataAsset`  
**Purpose:** Maps input keys to per-platform glyph textures.

```cpp
UCLASS(BlueprintType)
class GAMEUI_API UAtlasInputGlyphData : public UDataAsset
{
    UPROPERTY(EditDefaultsOnly)
    TMap<FKey, TSoftObjectPtr<UTexture2D>> KeyboardGlyphs;

    UPROPERTY(EditDefaultsOnly)
    TMap<FKey, TSoftObjectPtr<UTexture2D>> XboxGlyphs;

    UPROPERTY(EditDefaultsOnly)
    TMap<FKey, TSoftObjectPtr<UTexture2D>> PlayStationGlyphs;

    const UTexture2D* GetGlyph(FKey Key, EAtlasInputDevice Device) const;
};
```

---

## Asset Manager Registration

In `UAtlasAssetManager::StartInitialLoading()` and/or in `DefaultEngine.ini`:

```ini
[/Script/Engine.AssetManager]
+PrimaryAssetTypesToScan=(PrimaryAssetType="AtlasPawnData",AssetBaseClass=/Script/GameActors.AtlasPawnData,HasBlueprintClasses=False,IsEditorOnly=False,Directories=((Path="/Game/PawnData")),SpecificAssets=(),Rules=(Priority=-1,ChunkId=-1,bApplyRecursively=True,CookRule=Unknown))
+PrimaryAssetTypesToScan=(PrimaryAssetType="AtlasAbilitySet",AssetBaseClass=/Script/GameActors.AtlasAbilitySet,HasBlueprintClasses=False,IsEditorOnly=False,Directories=((Path="/Game/Abilities")),SpecificAssets=(),Rules=(Priority=-1,ChunkId=-1,bApplyRecursively=True,CookRule=Unknown))
+PrimaryAssetTypesToScan=(PrimaryAssetType="AtlasScreenDefinition",AssetBaseClass=/Script/GameUI.AtlasScreenDefinition,HasBlueprintClasses=False,IsEditorOnly=False,Directories=((Path="/Game/UI/Screens")),SpecificAssets=(),Rules=(Priority=-1,ChunkId=-1,bApplyRecursively=True,CookRule=Unknown))
```

---

## Summary: What Goes Where

| Configuration | Location | Rationale |
|---|---|---|
| Map soft references | `UAtlasDeveloperSettings` | Single per-project |
| Default pawn | `UAtlasDeveloperSettings` | Single per-project fallback |
| Per-pawn capabilities | `UAtlasPawnData` data asset | Multiple instances, per-pawn-type |
| Per-pawn abilities | `UAtlasAbilitySet` data asset | Multiple, composable |
| Input contexts per pawn | `UAtlasInputConfigData` data asset | Multiple, composable |
| UI screen definitions | `UAtlasScreenDefinition` data assets | Multiple, per-screen |
| All screen IDs → definitions | `UAtlasScreenRegistry` data asset | Single per-project |
| Camera config per pawn/mode | `UAtlasCameraConfig` data asset | Multiple, per-pawn-type |
| Input glyphs | `UAtlasInputGlyphData` data asset | Single per-project |
| Autosave settings | `UAtlasDeveloperSettings` | Single per-project |
| UI glyph settings | `UAtlasUIDeveloperSettings` | Single per-project |
