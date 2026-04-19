#pragma once

#include "CoreMinimal.h"
#include "Serialization/MemoryReader.h"
#include "Systems/Serialization/AtlasSaveHeader.h"
#include "Systems/WorldState/AtlasWorldSnapshotTypes.h"

/*
 * Binary reader for Atlas world snapshots.
 *
 * Uses FArchive internally and validates the versioned, chunked payload.
 */
class GAMECORE_API FAtlasBinaryReader
{
public:
	explicit FAtlasBinaryReader(const TArray<uint8>& InInputData);

	static bool Deserialize(const TArray<uint8>& InputData, FAtlasWorldSnapshot& OutWorldSnapshot);

	bool ReadSnapshot(FAtlasWorldSnapshot& OutWorldSnapshot);

	bool BeginChunk(FName ExpectedChunkName = NAME_None);
	void EndChunk();

	FArchive& GetArchive();
	const FAtlasSaveHeader& GetHeader() const;

private:
	bool ReadHeader();
	bool ReadWorldSnapshot(FAtlasWorldSnapshot& OutWorldSnapshot);
	bool ReadActorSnapshot(FAtlasActorSnapshot& OutActorSnapshot);
	bool ReadDataChunk(FAtlasDataChunk& OutDataChunk);
	bool HasArchiveError() const;

	const TArray<uint8>& InputData;
	FMemoryReader Archive;
	FAtlasSaveHeader Header;
	TArray<FName> ChunkNameStack;
	TArray<int64> ChunkEndOffsetStack;
};
