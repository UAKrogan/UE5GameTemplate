#pragma once

#include "CommonInputBaseTypes.h"
#include "Data/AtlasUITypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "AtlasGlyphSubsystem.generated.h"

class UAtlasInputGlyphData;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAtlasInputDeviceChanged, EAtlasInputDevice, NewDevice);

/*
 * Tracks the active input device for glyph display.
 *
 * Wraps Common Input's UCommonInputSubsystem: input method changes are mapped
 * to EAtlasInputDevice and rebroadcast, so glyph widgets never need to know
 * about Common Input directly.
 */
UCLASS()
class GAMEUI_API UAtlasGlyphSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Glyphs")
	EAtlasInputDevice GetCurrentInputDevice() const { return CurrentDevice; }

	/*
	 * Resolves the glyph texture for a key on the current input device using
	 * the glyph data asset from UAtlasUIDeveloperSettings.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Glyphs")
	UTexture2D* GetGlyphForKey(FKey Key) const;

	UPROPERTY(BlueprintAssignable, Category = "Atlas UI|Glyphs")
	FAtlasInputDeviceChanged OnInputDeviceChanged;

private:
	void HandleInputMethodChanged(ECommonInputType NewInputType);
	EAtlasInputDevice ResolveDevice(ECommonInputType InputType) const;
	const UAtlasInputGlyphData* ResolveGlyphData() const;

	EAtlasInputDevice CurrentDevice = EAtlasInputDevice::KeyboardMouse;
};
