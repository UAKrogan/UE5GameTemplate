#pragma once

#include "Data/AtlasUITypes.h"
#include "Engine/DataAsset.h"
#include "InputCoreTypes.h"
#include "AtlasInputGlyphData.generated.h"

/*
 * Maps input keys to per-platform glyph textures. One per project,
 * referenced by UAtlasUIDeveloperSettings.
 */
UCLASS(BlueprintType)
class GAMEUI_API UAtlasInputGlyphData : public UDataAsset
{
	GENERATED_BODY()

public:
	/*
	 * Resolves the glyph for a key on the given device, loading the soft
	 * reference synchronously. Falls back to keyboard glyphs for Generic.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Glyphs")
	UTexture2D* GetGlyph(FKey Key, EAtlasInputDevice Device) const;

	UPROPERTY(EditDefaultsOnly, Category = "Glyphs")
	TMap<FKey, TSoftObjectPtr<UTexture2D>> KeyboardGlyphs;

	UPROPERTY(EditDefaultsOnly, Category = "Glyphs")
	TMap<FKey, TSoftObjectPtr<UTexture2D>> XboxGlyphs;

	UPROPERTY(EditDefaultsOnly, Category = "Glyphs")
	TMap<FKey, TSoftObjectPtr<UTexture2D>> PlayStationGlyphs;
};
