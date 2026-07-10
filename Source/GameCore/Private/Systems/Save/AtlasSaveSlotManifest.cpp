#include "Systems/Save/AtlasSaveSlotManifest.h"

#include "Dom/JsonObject.h"
#include "Logging/AtlasLogMacros.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Systems/Storage/AtlasFileStorage.h"

namespace AtlasSaveSlotManifest
{
	const TCHAR* ManifestFileName = TEXT("SaveManifest.json");

	TSharedRef<FJsonObject> MetadataToJson(const FAtlasSaveGameMetadata& Metadata)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetNumberField(TEXT("DataVersion"), Metadata.DataVersion);
		Object->SetStringField(TEXT("SlotName"), Metadata.SlotName);
		Object->SetNumberField(TEXT("UserIndex"), Metadata.UserIndex);
		Object->SetStringField(TEXT("SaveId"), Metadata.SaveId);
		Object->SetStringField(TEXT("SavedAtUtc"), Metadata.SavedAtUtc);
		Object->SetStringField(TEXT("BuildVersion"), Metadata.BuildVersion);
		Object->SetStringField(TEXT("MapName"), Metadata.MapName);
		return Object;
	}

	FAtlasSaveGameMetadata JsonToMetadata(const TSharedPtr<FJsonObject>& Object)
	{
		FAtlasSaveGameMetadata Metadata;
		Object->TryGetNumberField(TEXT("DataVersion"), Metadata.DataVersion);
		Object->TryGetStringField(TEXT("SlotName"), Metadata.SlotName);
		Object->TryGetNumberField(TEXT("UserIndex"), Metadata.UserIndex);
		Object->TryGetStringField(TEXT("SaveId"), Metadata.SaveId);
		Object->TryGetStringField(TEXT("SavedAtUtc"), Metadata.SavedAtUtc);
		Object->TryGetStringField(TEXT("BuildVersion"), Metadata.BuildVersion);
		Object->TryGetStringField(TEXT("MapName"), Metadata.MapName);
		return Metadata;
	}
}

bool FAtlasSaveSlotManifest::AddSlot(const FString& SlotName, const FAtlasSaveGameMetadata& Metadata)
{
	TArray<FAtlasSaveGameMetadata> Slots;
	LoadManifest(Slots);

	Slots.RemoveAll([&SlotName](const FAtlasSaveGameMetadata& Existing)
	{
		return Existing.SlotName == SlotName;
	});
	Slots.Add(Metadata);

	return SaveManifest(Slots);
}

bool FAtlasSaveSlotManifest::RemoveSlot(const FString& SlotName)
{
	TArray<FAtlasSaveGameMetadata> Slots;
	if (!LoadManifest(Slots))
	{
		return false;
	}

	const int32 Removed = Slots.RemoveAll([&SlotName](const FAtlasSaveGameMetadata& Existing)
	{
		return Existing.SlotName == SlotName;
	});

	return Removed > 0 && SaveManifest(Slots);
}

bool FAtlasSaveSlotManifest::GetSlotMetadata(const FString& SlotName, FAtlasSaveGameMetadata& OutMetadata)
{
	TArray<FAtlasSaveGameMetadata> Slots;
	if (!LoadManifest(Slots))
	{
		return false;
	}

	for (const FAtlasSaveGameMetadata& Metadata : Slots)
	{
		if (Metadata.SlotName == SlotName)
		{
			OutMetadata = Metadata;
			return true;
		}
	}

	return false;
}

TArray<FAtlasSaveGameMetadata> FAtlasSaveSlotManifest::GetAllSlots()
{
	TArray<FAtlasSaveGameMetadata> Slots;
	LoadManifest(Slots);
	return Slots;
}

FString FAtlasSaveSlotManifest::GetManifestPath()
{
	return FAtlasFileStorage::GetSaveDirectory() / AtlasSaveSlotManifest::ManifestFileName;
}

bool FAtlasSaveSlotManifest::LoadManifest(TArray<FAtlasSaveGameMetadata>& OutSlots)
{
	OutSlots.Reset();

	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *GetManifestPath()))
	{
		// Missing manifest is normal on first run.
		return false;
	}

	TArray<TSharedPtr<FJsonValue>> JsonValues;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, JsonValues))
	{
		ATLAS_LOG_CORE(Warning, "Save manifest is corrupt: %s", *GetManifestPath());
		return false;
	}

	for (const TSharedPtr<FJsonValue>& Value : JsonValues)
	{
		const TSharedPtr<FJsonObject>* Object;
		if (Value.IsValid() && Value->TryGetObject(Object))
		{
			OutSlots.Add(AtlasSaveSlotManifest::JsonToMetadata(*Object));
		}
	}

	return true;
}

bool FAtlasSaveSlotManifest::SaveManifest(const TArray<FAtlasSaveGameMetadata>& Slots)
{
	TArray<TSharedPtr<FJsonValue>> JsonValues;
	JsonValues.Reserve(Slots.Num());
	for (const FAtlasSaveGameMetadata& Metadata : Slots)
	{
		JsonValues.Add(MakeShared<FJsonValueObject>(AtlasSaveSlotManifest::MetadataToJson(Metadata)));
	}

	FString JsonText;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonText);
	if (!FJsonSerializer::Serialize(JsonValues, Writer))
	{
		return false;
	}

	if (!FFileHelper::SaveStringToFile(JsonText, *GetManifestPath()))
	{
		ATLAS_LOG_CORE(Warning, "Failed to write save manifest: %s", *GetManifestPath());
		return false;
	}

	return true;
}
