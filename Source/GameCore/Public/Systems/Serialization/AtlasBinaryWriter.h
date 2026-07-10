#pragma once

#include "CoreMinimal.h"
#include "Serialization/MemoryWriter.h"
#include "Systems/Serialization/AtlasSaveHeader.h"
#include "Systems/WorldState/AtlasWorldSnapshotTypes.h"

/*
 * Well-known chunk names shared by the binary writer and reader.
 */
namespace AtlasBinarySerialization
{
	inline const FName HeaderChunkName = TEXT("Header");
	inline const FName WorldSnapshotChunkName = TEXT("WorldSnapshot");
	inline const FName ActorSnapshotChunkName = TEXT("ActorSnapshot");
	inline const FName DataChunkName = TEXT("DataChunk");
}

/*
 * Binary writer for Atlas world snapshots.
 *
 * Uses FArchive internally and writes a versioned, chunked payload to memory.
 */
class GAMECORE_API FAtlasBinaryWriter
{
public:
	static constexpr int32 CurrentVersion = 1;

	explicit FAtlasBinaryWriter(TArray<uint8>& InOutputData);

	static TArray<uint8> Serialize(const FAtlasWorldSnapshot& WorldSnapshot);
	static bool Serialize(const FAtlasWorldSnapshot& WorldSnapshot, TArray<uint8>& OutData);

	bool WriteSnapshot(const FAtlasWorldSnapshot& WorldSnapshot);

	void BeginChunk(FName ChunkName);
	void EndChunk();

	FArchive& GetArchive();
	const FAtlasSaveHeader& GetHeader() const;

private:
	void WriteHeader();
	void WriteWorldSnapshot(const FAtlasWorldSnapshot& WorldSnapshot);
	void WriteActorSnapshot(const FAtlasActorSnapshot& ActorSnapshot);
	void WriteDataChunk(const FAtlasDataChunk& DataChunk);

	TArray<uint8>& OutputData;
	FMemoryWriter Archive;
	FAtlasSaveHeader Header;
	TArray<FName> ChunkNameStack;
	TArray<int64> ChunkSizeOffsetStack;
	TArray<int64> ChunkDataStartOffsetStack;
};
