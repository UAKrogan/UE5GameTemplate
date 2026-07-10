#include "Subsystems/AtlasGlyphSubsystem.h"

#include "CommonInputSubsystem.h"
#include "Data/AtlasInputGlyphData.h"
#include "Logging/AtlasLogMacros.h"
#include "Settings/AtlasUIDeveloperSettings.h"

void UAtlasGlyphSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!UAtlasUIDeveloperSettings::Get()->bAutoDetectInputDevice)
	{
		return;
	}

	if (UCommonInputSubsystem* CommonInput = Collection.InitializeDependency<UCommonInputSubsystem>())
	{
		CommonInput->OnInputMethodChangedNative.AddUObject(this, &UAtlasGlyphSubsystem::HandleInputMethodChanged);
		CurrentDevice = ResolveDevice(CommonInput->GetCurrentInputType());
	}
}

void UAtlasGlyphSubsystem::Deinitialize()
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UCommonInputSubsystem* CommonInput = LocalPlayer->GetSubsystem<UCommonInputSubsystem>())
		{
			CommonInput->OnInputMethodChangedNative.RemoveAll(this);
		}
	}

	Super::Deinitialize();
}

UTexture2D* UAtlasGlyphSubsystem::GetGlyphForKey(FKey Key) const
{
	if (const UAtlasInputGlyphData* GlyphData = ResolveGlyphData())
	{
		return GlyphData->GetGlyph(Key, CurrentDevice);
	}

	return nullptr;
}

void UAtlasGlyphSubsystem::HandleInputMethodChanged(ECommonInputType NewInputType)
{
	const EAtlasInputDevice NewDevice = ResolveDevice(NewInputType);
	if (NewDevice == CurrentDevice)
	{
		return;
	}

	CurrentDevice = NewDevice;
	OnInputDeviceChanged.Broadcast(CurrentDevice);

	ATLAS_LOG_UI(Log, "Input device changed: %d", static_cast<int32>(CurrentDevice));
}

EAtlasInputDevice UAtlasGlyphSubsystem::ResolveDevice(ECommonInputType InputType) const
{
	if (InputType != ECommonInputType::Gamepad)
	{
		return EAtlasInputDevice::KeyboardMouse;
	}

	FName GamepadName = NAME_None;
	if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (const UCommonInputSubsystem* CommonInput = LocalPlayer->GetSubsystem<UCommonInputSubsystem>())
		{
			GamepadName = CommonInput->GetCurrentGamepadName();
		}
	}

	const FString GamepadString = GamepadName.ToString();
	if (GamepadString.Contains(TEXT("PS")) || GamepadString.Contains(TEXT("Dual")))
	{
		return EAtlasInputDevice::PlayStation;
	}
	if (GamepadString.Contains(TEXT("Xbox")) || GamepadString.Contains(TEXT("XSX")) || GamepadString.Contains(TEXT("XB")))
	{
		return EAtlasInputDevice::Xbox;
	}

	return EAtlasInputDevice::Generic;
}

const UAtlasInputGlyphData* UAtlasGlyphSubsystem::ResolveGlyphData() const
{
	return UAtlasUIDeveloperSettings::Get()->InputGlyphData.LoadSynchronous();
}
