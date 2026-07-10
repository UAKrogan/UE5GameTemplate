#include "Data/AtlasInputGlyphData.h"

UTexture2D* UAtlasInputGlyphData::GetGlyph(FKey Key, EAtlasInputDevice Device) const
{
	const TMap<FKey, TSoftObjectPtr<UTexture2D>>* GlyphMap;
	switch (Device)
	{
	case EAtlasInputDevice::Xbox:
		GlyphMap = &XboxGlyphs;
		break;
	case EAtlasInputDevice::PlayStation:
		GlyphMap = &PlayStationGlyphs;
		break;
	case EAtlasInputDevice::Generic:
		// Generic gamepads use the Xbox set as the least surprising default.
		GlyphMap = &XboxGlyphs;
		break;
	default:
		GlyphMap = &KeyboardGlyphs;
		break;
	}

	if (const TSoftObjectPtr<UTexture2D>* Glyph = GlyphMap->Find(Key))
	{
		return Glyph->LoadSynchronous();
	}

	return nullptr;
}
