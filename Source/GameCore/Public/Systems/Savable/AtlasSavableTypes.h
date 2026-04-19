#pragma once

#include "CoreMinimal.h"

/*
 * Archive-backed save context used by savable objects when writing state.
 *
 * Supports nested chunk serialization with explicit BeginChunk / EndChunk calls.
 */
struct GAMECORE_API FAtlasSaveContext
{
public:
	explicit FAtlasSaveContext(FArchive& InArchive);

	void BeginChunk(FName ChunkId);
	void EndChunk();

	FArchive& GetArchive();
	const FArchive& GetArchive() const;

	bool IsSaving() const;
	bool IsInChunk() const;
	FName GetCurrentChunkId() const;

	template <typename TValue>
	void Serialize(TValue& Value)
	{
		GetArchive() << Value;
	}

	void SerializeBytes(void* Data, int64 NumBytes);

private:
	FArchive* Archive = nullptr;
	TArray<FName> ChunkIdStack;
	TArray<int64> SizeOffsetStack;
	TArray<int64> DataStartOffsetStack;
};

/*
 * Archive-backed load context used by savable objects when restoring state.
 *
 * Supports nested chunk deserialization with explicit BeginChunk / EndChunk calls.
 */
struct GAMECORE_API FAtlasLoadContext
{
public:
	explicit FAtlasLoadContext(FArchive& InArchive);

	bool BeginChunk(FName ExpectedChunkId = NAME_None) const;
	void EndChunk() const;

	FArchive& GetArchive();
	const FArchive& GetArchive() const;

	bool IsLoading() const;
	bool IsInChunk() const;
	bool HasMoreDataInChunk() const;
	FName GetCurrentChunkId() const;

	template <typename TValue>
	void Serialize(TValue& Value) const
	{
		const_cast<FAtlasLoadContext*>(this)->GetArchive() << Value;
	}

	void SerializeBytes(void* Data, int64 NumBytes) const;

private:
	mutable FArchive* Archive = nullptr;
	mutable TArray<FName> ChunkIdStack;
	mutable TArray<int64> ChunkEndOffsetStack;
};
