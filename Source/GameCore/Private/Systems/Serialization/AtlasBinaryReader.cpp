#include "Systems/Serialization/AtlasBinaryReader.h"

#include "Systems/Serialization/AtlasBinaryWriter.h"
#include "Logging/AtlasLogMacros.h"

namespace AtlasBinarySerialization
{
	const FName HeaderChunkName = TEXT("Header");
	const FName WorldSnapshotChunkName = TEXT("WorldSnapshot");
	const FName ActorSnapshotChunkName = TEXT("ActorSnapshot");
	const FName DataChunkName = TEXT("DataChunk");
}

FAtlasBinaryReader::FAtlasBinaryReader(const TArray<uint8>& InInputData)
	: InputData(InInputData)
	, Archive(InputData, true)
{
}

bool FAtlasBinaryReader::Deserialize(const TArray<uint8>& InputData, FAtlasWorldSnapshot& OutWorldSnapshot)
{
	FAtlasBinaryReader Reader(InputData);
	return Reader.ReadSnapshot(OutWorldSnapshot);
}

bool FAtlasBinaryReader::ReadSnapshot(FAtlasWorldSnapshot& OutWorldSnapshot)
{
	OutWorldSnapshot.Reset();

	if (InputData.IsEmpty())
	{
		ATLAS_LOG_CORE(Warning, "Failed to deserialize Atlas world snapshot: input data is empty");
		return false;
	}

	if (!ReadHeader())
	{
		return false;
	}

	if (!Header.IsSupportedVersion(FAtlasBinaryWriter::CurrentVersion))
	{
		ATLAS_LOG_CORE(Error, "Unsupported Atlas save version: version=%d current=%d",
			Header.Version,
			FAtlasBinaryWriter::CurrentVersion);
		return false;
	}

	if (!ReadWorldSnapshot(OutWorldSnapshot))
	{
		OutWorldSnapshot.Reset();
		return false;
	}

	return !HasArchiveError() && ChunkNameStack.IsEmpty();
}

bool FAtlasBinaryReader::BeginChunk(FName ExpectedChunkName)
{
	FName ActualChunkName = NAME_None;
	int64 ChunkSizeBytes = 0;

	Archive << ActualChunkName;
	Archive << ChunkSizeBytes;

	if (HasArchiveError() || ActualChunkName.IsNone() || ChunkSizeBytes < 0)
	{
		ATLAS_LOG_CORE(Error, "Invalid Atlas binary chunk header");
		return false;
	}

	if (ExpectedChunkName != NAME_None && ActualChunkName != ExpectedChunkName)
	{
		ATLAS_LOG_CORE(Error, "Unexpected Atlas binary chunk. Expected=%s Actual=%s",
			*ExpectedChunkName.ToString(),
			*ActualChunkName.ToString());
		return false;
	}

	const int64 ChunkEndOffset = Archive.Tell() + ChunkSizeBytes;
	if (ChunkEndOffset > InputData.Num())
	{
		ATLAS_LOG_CORE(Error, "Invalid Atlas binary chunk size: chunk=%s size=%lld",
			*ActualChunkName.ToString(),
			ChunkSizeBytes);
		return false;
	}

	ChunkNameStack.Add(ActualChunkName);
	ChunkEndOffsetStack.Add(ChunkEndOffset);
	return true;
}

void FAtlasBinaryReader::EndChunk()
{
	check(ChunkNameStack.Num() > 0);
	check(ChunkEndOffsetStack.Num() == ChunkNameStack.Num());

	const int32 StackIndex = ChunkNameStack.Num() - 1;
	const int64 ChunkEndOffset = ChunkEndOffsetStack[StackIndex];

	if (Archive.Tell() < ChunkEndOffset)
	{
		Archive.Seek(ChunkEndOffset);
	}

	ChunkNameStack.Pop();
	ChunkEndOffsetStack.Pop();
}

FArchive& FAtlasBinaryReader::GetArchive()
{
	return Archive;
}

const FAtlasSaveHeader& FAtlasBinaryReader::GetHeader() const
{
	return Header;
}

bool FAtlasBinaryReader::ReadHeader()
{
	if (!BeginChunk(AtlasBinarySerialization::HeaderChunkName))
	{
		return false;
	}

	Archive << Header.Version;

	int64 TimestampTicks = 0;
	Archive << TimestampTicks;
	Header.Timestamp = FDateTime(TimestampTicks);

	EndChunk();
	return !HasArchiveError();
}

bool FAtlasBinaryReader::ReadWorldSnapshot(FAtlasWorldSnapshot& OutWorldSnapshot)
{
	if (!BeginChunk(AtlasBinarySerialization::WorldSnapshotChunkName))
	{
		return false;
	}

	int32 ActorCount = 0;
	Archive << ActorCount;

	if (ActorCount < 0)
	{
		ATLAS_LOG_CORE(Error, "Invalid Atlas actor snapshot count: %d", ActorCount);
		EndChunk();
		return false;
	}

	OutWorldSnapshot.Actors.Reserve(ActorCount);
	for (int32 ActorIndex = 0; ActorIndex < ActorCount; ++ActorIndex)
	{
		FAtlasActorSnapshot ActorSnapshot;
		if (!ReadActorSnapshot(ActorSnapshot))
		{
			EndChunk();
			return false;
		}

		OutWorldSnapshot.Actors.Add(MoveTemp(ActorSnapshot));
	}

	EndChunk();
	return !HasArchiveError();
}

bool FAtlasBinaryReader::ReadActorSnapshot(FAtlasActorSnapshot& OutActorSnapshot)
{
	if (!BeginChunk(AtlasBinarySerialization::ActorSnapshotChunkName))
	{
		return false;
	}

	int32 DataChunkCount = 0;

	Archive << OutActorSnapshot.ActorId;
	Archive << OutActorSnapshot.ClassPath;
	Archive << OutActorSnapshot.Transform;
	Archive << DataChunkCount;

	if (DataChunkCount < 0)
	{
		ATLAS_LOG_CORE(Error, "Invalid Atlas data chunk count: %d", DataChunkCount);
		EndChunk();
		return false;
	}

	OutActorSnapshot.Chunks.Reserve(DataChunkCount);
	for (int32 ChunkIndex = 0; ChunkIndex < DataChunkCount; ++ChunkIndex)
	{
		FAtlasDataChunk DataChunk;
		if (!ReadDataChunk(DataChunk))
		{
			EndChunk();
			return false;
		}

		OutActorSnapshot.Chunks.Add(MoveTemp(DataChunk));
	}

	EndChunk();
	return !HasArchiveError();
}

bool FAtlasBinaryReader::ReadDataChunk(FAtlasDataChunk& OutDataChunk)
{
	if (!BeginChunk(AtlasBinarySerialization::DataChunkName))
	{
		return false;
	}

	int32 DataSize = 0;
	Archive << OutDataChunk.Name;
	Archive << DataSize;

	if (DataSize < 0)
	{
		ATLAS_LOG_CORE(Error, "Invalid Atlas data chunk size: %d", DataSize);
		EndChunk();
		return false;
	}

	OutDataChunk.Data.SetNumUninitialized(DataSize);
	if (DataSize > 0)
	{
		Archive.Serialize(OutDataChunk.Data.GetData(), DataSize);
	}

	EndChunk();
	return !HasArchiveError();
}

bool FAtlasBinaryReader::HasArchiveError() const
{
	return Archive.IsError() || Archive.IsCriticalError();
}
