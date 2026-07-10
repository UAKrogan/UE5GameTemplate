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

	/*
	 * One-shot convenience: reads a full snapshot from a raw buffer.
	 */
	static bool Deserialize(const TArray<uint8>& InputData, FAtlasWorldSnapshot& OutWorldSnapshot);

	/*
	 * Reads header + world snapshot, then runs registered save migrations
	 * when the payload version is older than the current format version.
	 */
	bool ReadSnapshot(FAtlasWorldSnapshot& OutWorldSnapshot);

	/*
	 * Validates and enters the next chunk. Chunk sizes let EndChunk skip any
	 * unread bytes, which is what makes unknown/extended chunks forward-safe.
	 */
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
