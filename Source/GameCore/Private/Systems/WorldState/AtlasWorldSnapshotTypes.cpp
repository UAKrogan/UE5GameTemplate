#include "Systems/WorldState/AtlasWorldSnapshotTypes.h"

FAtlasDataChunk& FAtlasActorSnapshot::AddChunk(FName ChunkName)
{
	check(!ChunkName.IsNone());

	if (FAtlasDataChunk* ExistingChunk = FindChunk(ChunkName))
	{
		ExistingChunk->Data.Reset();
		return *ExistingChunk;
	}

	return Chunks.Emplace_GetRef(ChunkName, TArray<uint8>());
}

FAtlasDataChunk& FAtlasActorSnapshot::AddChunk(FName ChunkName, const TArray<uint8>& ChunkData)
{
	FAtlasDataChunk& Chunk = AddChunk(ChunkName);
	Chunk.Data = ChunkData;
	return Chunk;
}

FAtlasDataChunk& FAtlasActorSnapshot::AddChunk(FName ChunkName, TArray<uint8>&& ChunkData)
{
	FAtlasDataChunk& Chunk = AddChunk(ChunkName);
	Chunk.Data = MoveTemp(ChunkData);
	return Chunk;
}

FAtlasDataChunk* FAtlasActorSnapshot::FindChunk(FName ChunkName)
{
	return Chunks.FindByPredicate([ChunkName](const FAtlasDataChunk& Chunk)
	{
		return Chunk.Name == ChunkName;
	});
}

const FAtlasDataChunk* FAtlasActorSnapshot::FindChunk(FName ChunkName) const
{
	return Chunks.FindByPredicate([ChunkName](const FAtlasDataChunk& Chunk)
	{
		return Chunk.Name == ChunkName;
	});
}

bool FAtlasActorSnapshot::RemoveChunk(FName ChunkName)
{
	const int32 RemovedCount = Chunks.RemoveAll([ChunkName](const FAtlasDataChunk& Chunk)
	{
		return Chunk.Name == ChunkName;
	});

	return RemovedCount > 0;
}

void FAtlasActorSnapshot::ResetChunks()
{
	Chunks.Reset();
}

bool FAtlasActorSnapshot::HasValidActorId() const
{
	return ActorId.IsValid();
}

void FAtlasWorldSnapshot::Reset()
{
	Actors.Reset();
}

bool FAtlasWorldSnapshot::IsEmpty() const
{
	return Actors.IsEmpty();
}

FAtlasActorSnapshot& FAtlasWorldSnapshot::AddActor(const FGuid& ActorId, const FString& ClassPath, const FTransform& Transform)
{
	check(ActorId.IsValid());

	if (FAtlasActorSnapshot* ExistingActor = FindActor(ActorId))
	{
		ExistingActor->ClassPath = ClassPath;
		ExistingActor->Transform = Transform;
		ExistingActor->ResetChunks();
		return *ExistingActor;
	}

	FAtlasActorSnapshot& ActorSnapshot = Actors.Emplace_GetRef();
	ActorSnapshot.ActorId = ActorId;
	ActorSnapshot.ClassPath = ClassPath;
	ActorSnapshot.Transform = Transform;
	return ActorSnapshot;
}

FAtlasActorSnapshot* FAtlasWorldSnapshot::FindActor(const FGuid& ActorId)
{
	return Actors.FindByPredicate([ActorId](const FAtlasActorSnapshot& ActorSnapshot)
	{
		return ActorSnapshot.ActorId == ActorId;
	});
}

const FAtlasActorSnapshot* FAtlasWorldSnapshot::FindActor(const FGuid& ActorId) const
{
	return Actors.FindByPredicate([ActorId](const FAtlasActorSnapshot& ActorSnapshot)
	{
		return ActorSnapshot.ActorId == ActorId;
	});
}

bool FAtlasWorldSnapshot::RemoveActor(const FGuid& ActorId)
{
	const int32 RemovedCount = Actors.RemoveAll([ActorId](const FAtlasActorSnapshot& ActorSnapshot)
	{
		return ActorSnapshot.ActorId == ActorId;
	});

	return RemovedCount > 0;
}
