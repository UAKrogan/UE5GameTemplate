#include "Systems/Savable/AtlasSavableTypes.h"

#include "Logging/AtlasLogMacros.h"
#include "Serialization/Archive.h"

namespace AtlasSavable::Private
{
	void ValidateArchive(FArchive* Archive)
	{
		check(Archive != nullptr);
		check(Archive->Tell() != INDEX_NONE);
	}
}

FAtlasSaveContext::FAtlasSaveContext(FArchive& InArchive)
	: Archive(&InArchive)
{
	check(Archive->IsSaving());
	AtlasSavable::Private::ValidateArchive(Archive);
}

void FAtlasSaveContext::BeginChunk(FName ChunkId)
{
	check(!ChunkId.IsNone());

	FArchive& LocalArchive = GetArchive();
	LocalArchive << ChunkId;

	const int64 SizeOffset = LocalArchive.Tell();
	int64 ChunkSizeBytes = 0;
	LocalArchive << ChunkSizeBytes;

	ChunkIdStack.Add(ChunkId);
	SizeOffsetStack.Add(SizeOffset);
	DataStartOffsetStack.Add(LocalArchive.Tell());
}

void FAtlasSaveContext::EndChunk()
{
	check(ChunkIdStack.Num() > 0);
	check(SizeOffsetStack.Num() == ChunkIdStack.Num());
	check(DataStartOffsetStack.Num() == ChunkIdStack.Num());

	FArchive& LocalArchive = GetArchive();
	const int32 StackIndex = ChunkIdStack.Num() - 1;
	const int64 EndOffset = LocalArchive.Tell();
	const int64 DataStartOffset = DataStartOffsetStack[StackIndex];
	const int64 ChunkSizeBytes = EndOffset - DataStartOffset;
	const int64 SizeOffset = SizeOffsetStack[StackIndex];

	LocalArchive.Seek(SizeOffset);
	int64 SerializedChunkSizeBytes = ChunkSizeBytes;
	LocalArchive << SerializedChunkSizeBytes;
	LocalArchive.Seek(EndOffset);

	ChunkIdStack.Pop();
	SizeOffsetStack.Pop();
	DataStartOffsetStack.Pop();
}

FArchive& FAtlasSaveContext::GetArchive()
{
	AtlasSavable::Private::ValidateArchive(Archive);
	return *Archive;
}

const FArchive& FAtlasSaveContext::GetArchive() const
{
	AtlasSavable::Private::ValidateArchive(Archive);
	return *Archive;
}

bool FAtlasSaveContext::IsSaving() const
{
	return Archive != nullptr && Archive->IsSaving();
}

bool FAtlasSaveContext::IsInChunk() const
{
	return ChunkIdStack.Num() > 0;
}

FName FAtlasSaveContext::GetCurrentChunkId() const
{
	return ChunkIdStack.Num() > 0 ? ChunkIdStack.Last() : NAME_None;
}

void FAtlasSaveContext::SerializeBytes(void* Data, int64 NumBytes)
{
	GetArchive().Serialize(Data, NumBytes);
}

FAtlasLoadContext::FAtlasLoadContext(FArchive& InArchive)
	: Archive(&InArchive)
{
	check(Archive->IsLoading());
	AtlasSavable::Private::ValidateArchive(Archive);
}

bool FAtlasLoadContext::BeginChunk(FName ExpectedChunkId) const
{
	FArchive& LocalArchive = const_cast<FAtlasLoadContext*>(this)->GetArchive();

	FName ActualChunkId = NAME_None;
	int64 ChunkSizeBytes = 0;
	LocalArchive << ActualChunkId;
	LocalArchive << ChunkSizeBytes;

	if (ActualChunkId.IsNone() || ChunkSizeBytes < 0)
	{
		ATLAS_LOG_CORE(Error, "Invalid Atlas save chunk header");
		return false;
	}

	if (ExpectedChunkId != NAME_None && ActualChunkId != ExpectedChunkId)
	{
		ATLAS_LOG_CORE(Error, "Unexpected Atlas save chunk. Expected=%s Actual=%s",
			*ExpectedChunkId.ToString(),
			*ActualChunkId.ToString());
		return false;
	}

	const int64 ChunkEndOffset = LocalArchive.Tell() + ChunkSizeBytes;
	ChunkIdStack.Add(ActualChunkId);
	ChunkEndOffsetStack.Add(ChunkEndOffset);
	return true;
}

void FAtlasLoadContext::EndChunk() const
{
	check(ChunkIdStack.Num() > 0);
	check(ChunkEndOffsetStack.Num() == ChunkIdStack.Num());

	FArchive& LocalArchive = const_cast<FAtlasLoadContext*>(this)->GetArchive();
	const int32 StackIndex = ChunkIdStack.Num() - 1;
	const int64 ChunkEndOffset = ChunkEndOffsetStack[StackIndex];

	if (LocalArchive.Tell() < ChunkEndOffset)
	{
		LocalArchive.Seek(ChunkEndOffset);
	}

	ChunkIdStack.Pop();
	ChunkEndOffsetStack.Pop();
}

FArchive& FAtlasLoadContext::GetArchive()
{
	AtlasSavable::Private::ValidateArchive(Archive);
	return *Archive;
}

const FArchive& FAtlasLoadContext::GetArchive() const
{
	AtlasSavable::Private::ValidateArchive(Archive);
	return *Archive;
}

bool FAtlasLoadContext::IsLoading() const
{
	return Archive != nullptr && Archive->IsLoading();
}

bool FAtlasLoadContext::IsInChunk() const
{
	return ChunkIdStack.Num() > 0;
}

bool FAtlasLoadContext::HasMoreDataInChunk() const
{
	if (ChunkEndOffsetStack.IsEmpty() || Archive == nullptr)
	{
		return false;
	}

	return Archive->Tell() < ChunkEndOffsetStack.Last();
}

FName FAtlasLoadContext::GetCurrentChunkId() const
{
	return ChunkIdStack.Num() > 0 ? ChunkIdStack.Last() : NAME_None;
}

void FAtlasLoadContext::SerializeBytes(void* Data, int64 NumBytes) const
{
	const_cast<FAtlasLoadContext*>(this)->GetArchive().Serialize(Data, NumBytes);
}
