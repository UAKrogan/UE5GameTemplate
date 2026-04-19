#pragma once

#include "CoreMinimal.h"
#include "AtlasWorldSnapshotTypes.generated.h"

/*
 * Raw serialized data for one named piece of actor state.
 *
 * Chunk names are intentionally caller-defined so actors and components can
 * store independent payloads inside a single actor snapshot.
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FAtlasDataChunk
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "WorldState")
	FName Name;

	UPROPERTY(VisibleAnywhere, Category = "WorldState")
	TArray<uint8> Data;

	FAtlasDataChunk() = default;

	FAtlasDataChunk(FName InName, const TArray<uint8>& InData)
		: Name(InName)
		, Data(InData)
	{
	}

	FAtlasDataChunk(FName InName, TArray<uint8>&& InData)
		: Name(InName)
		, Data(MoveTemp(InData))
	{
	}

	bool IsValid() const
	{
		return !Name.IsNone();
	}
};

/*
 * Pure data snapshot for a single actor.
 *
 * Does not hold UObject, AActor, UClass, UActorComponent, or level references.
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FAtlasActorSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "WorldState")
	FGuid ActorId;

	UPROPERTY(VisibleAnywhere, Category = "WorldState")
	FString ClassPath;

	UPROPERTY(VisibleAnywhere, Category = "WorldState")
	FTransform Transform = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, Category = "WorldState")
	TArray<FAtlasDataChunk> Chunks;

	FAtlasDataChunk& AddChunk(FName ChunkName);
	FAtlasDataChunk& AddChunk(FName ChunkName, const TArray<uint8>& ChunkData);
	FAtlasDataChunk& AddChunk(FName ChunkName, TArray<uint8>&& ChunkData);

	FAtlasDataChunk* FindChunk(FName ChunkName);
	const FAtlasDataChunk* FindChunk(FName ChunkName) const;

	bool RemoveChunk(FName ChunkName);
	void ResetChunks();
	bool HasValidActorId() const;
};

/*
 * Pure data snapshot for the saved world state.
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FAtlasWorldSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "WorldState")
	TArray<FAtlasActorSnapshot> Actors;

	void Reset();
	bool IsEmpty() const;

	FAtlasActorSnapshot& AddActor(const FGuid& ActorId, const FString& ClassPath, const FTransform& Transform);
	FAtlasActorSnapshot* FindActor(const FGuid& ActorId);
	const FAtlasActorSnapshot* FindActor(const FGuid& ActorId) const;
	bool RemoveActor(const FGuid& ActorId);
};
