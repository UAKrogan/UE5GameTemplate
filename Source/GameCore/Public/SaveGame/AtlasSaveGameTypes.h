#pragma once

#include "CoreMinimal.h"
#include "AtlasSaveGameTypes.generated.h"

USTRUCT(BlueprintType)
struct GAMECORE_API FAtlasSaveGameMetadata
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	int32 DataVersion = 1;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	FString SlotName;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	int32 UserIndex = 0;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	FString SaveId;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	FString SavedAtUtc;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	FString BuildVersion;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	FString MapName;

	bool IsValid() const
	{
		return !SlotName.IsEmpty();
	}
};

USTRUCT(BlueprintType)
struct GAMECORE_API FAtlasSaveGameSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	FAtlasSaveGameMetadata Metadata;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	TMap<FName, int32> IntValues;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	TMap<FName, float> FloatValues;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	TMap<FName, FString> StringValues;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	TMap<FName, FVector> VectorValues;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	TMap<FName, FRotator> RotatorValues;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	TMap<FName, FTransform> TransformValues;

	void Reset()
	{
		Metadata = FAtlasSaveGameMetadata();
		IntValues.Reset();
		FloatValues.Reset();
		StringValues.Reset();
		VectorValues.Reset();
		RotatorValues.Reset();
		TransformValues.Reset();
	}

	bool IsEmpty() const
	{
		return IntValues.IsEmpty()
			&& FloatValues.IsEmpty()
			&& StringValues.IsEmpty()
			&& VectorValues.IsEmpty()
			&& RotatorValues.IsEmpty()
			&& TransformValues.IsEmpty();
	}
};
