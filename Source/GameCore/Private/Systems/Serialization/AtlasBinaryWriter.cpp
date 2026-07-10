#include "Systems/Serialization/AtlasBinaryWriter.h"

#include "Logging/AtlasLogMacros.h"

FAtlasBinaryWriter::FAtlasBinaryWriter(TArray<uint8>& InOutputData)
	: OutputData(InOutputData)
	, Archive(OutputData, true)
{
	OutputData.Reset();
	Header.Version = CurrentVersion;
	Header.Timestamp = FDateTime::UtcNow();
}

TArray<uint8> FAtlasBinaryWriter::Serialize(const FAtlasWorldSnapshot& WorldSnapshot)
{
	TArray<uint8> OutputData;
	Serialize(WorldSnapshot, OutputData);
	return OutputData;
}

bool FAtlasBinaryWriter::Serialize(const FAtlasWorldSnapshot& WorldSnapshot, TArray<uint8>& OutData)
{
	FAtlasBinaryWriter Writer(OutData);
	return Writer.WriteSnapshot(WorldSnapshot);
}

bool FAtlasBinaryWriter::WriteSnapshot(const FAtlasWorldSnapshot& WorldSnapshot)
{
	WriteHeader();
	WriteWorldSnapshot(WorldSnapshot);

	const bool bSuccess = !Archive.IsError() && ChunkNameStack.IsEmpty();
	if (!bSuccess)
	{
		ATLAS_LOG_CORE(Error, "Failed to serialize Atlas world snapshot");
	}

	return bSuccess;
}

void FAtlasBinaryWriter::BeginChunk(FName ChunkName)
{
	check(!ChunkName.IsNone());

	Archive << ChunkName;

	const int64 SizeOffset = Archive.Tell();
	int64 ChunkSizeBytes = 0;
	Archive << ChunkSizeBytes;

	ChunkNameStack.Add(ChunkName);
	ChunkSizeOffsetStack.Add(SizeOffset);
	ChunkDataStartOffsetStack.Add(Archive.Tell());
}

void FAtlasBinaryWriter::EndChunk()
{
	check(ChunkNameStack.Num() > 0);
	check(ChunkSizeOffsetStack.Num() == ChunkNameStack.Num());
	check(ChunkDataStartOffsetStack.Num() == ChunkNameStack.Num());

	const int32 StackIndex = ChunkNameStack.Num() - 1;
	const int64 EndOffset = Archive.Tell();
	const int64 ChunkDataStartOffset = ChunkDataStartOffsetStack[StackIndex];
	const int64 ChunkSizeBytes = EndOffset - ChunkDataStartOffset;
	const int64 ChunkSizeOffset = ChunkSizeOffsetStack[StackIndex];

	Archive.Seek(ChunkSizeOffset);
	int64 SerializedChunkSizeBytes = ChunkSizeBytes;
	Archive << SerializedChunkSizeBytes;
	Archive.Seek(EndOffset);

	ChunkNameStack.Pop();
	ChunkSizeOffsetStack.Pop();
	ChunkDataStartOffsetStack.Pop();
}

FArchive& FAtlasBinaryWriter::GetArchive()
{
	return Archive;
}

const FAtlasSaveHeader& FAtlasBinaryWriter::GetHeader() const
{
	return Header;
}

void FAtlasBinaryWriter::WriteHeader()
{
	BeginChunk(AtlasBinarySerialization::HeaderChunkName);

	Archive << Header.Version;

	int64 TimestampTicks = Header.Timestamp.GetTicks();
	Archive << TimestampTicks;

	EndChunk();
}

void FAtlasBinaryWriter::WriteWorldSnapshot(const FAtlasWorldSnapshot& WorldSnapshot)
{
	BeginChunk(AtlasBinarySerialization::WorldSnapshotChunkName);

	int32 ActorCount = WorldSnapshot.Actors.Num();
	Archive << ActorCount;

	for (const FAtlasActorSnapshot& ActorSnapshot : WorldSnapshot.Actors)
	{
		WriteActorSnapshot(ActorSnapshot);
	}

	EndChunk();
}

void FAtlasBinaryWriter::WriteActorSnapshot(const FAtlasActorSnapshot& ActorSnapshot)
{
	BeginChunk(AtlasBinarySerialization::ActorSnapshotChunkName);

	FGuid ActorId = ActorSnapshot.ActorId;
	FString ClassPath = ActorSnapshot.ClassPath;
	FTransform Transform = ActorSnapshot.Transform;
	int32 ChunkCount = ActorSnapshot.Chunks.Num();

	Archive << ActorId;
	Archive << ClassPath;
	Archive << Transform;
	Archive << ChunkCount;

	for (const FAtlasDataChunk& DataChunk : ActorSnapshot.Chunks)
	{
		WriteDataChunk(DataChunk);
	}

	EndChunk();
}

void FAtlasBinaryWriter::WriteDataChunk(const FAtlasDataChunk& DataChunk)
{
	BeginChunk(AtlasBinarySerialization::DataChunkName);

	FName ChunkName = DataChunk.Name;
	int32 DataSize = DataChunk.Data.Num();

	Archive << ChunkName;
	Archive << DataSize;

	if (DataSize > 0)
	{
		Archive.Serialize(const_cast<uint8*>(DataChunk.Data.GetData()), DataSize);
	}

	EndChunk();
}
