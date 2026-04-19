#include "Systems/Save/AtlasSaveCollector.h"

#include "Systems/Savable/AtlasSavable.h"
#include "Systems/Savable/AtlasSavableComponent.h"

#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Logging/AtlasLogMacros.h"
#include "Serialization/MemoryWriter.h"

namespace AtlasSaveCollector
{
	const FName ActorChunkName = TEXT("Actor");
	const TCHAR* ComponentChunkPrefix = TEXT("Component");
}

FAtlasWorldSnapshot FAtlasSaveCollector::CollectWorld(UWorld* World)
{
	check(IsInGameThread());

	FAtlasWorldSnapshot WorldSnapshot;

	if (!World)
	{
		ATLAS_LOG_CORE(Warning, "Save collection skipped: world is null");
		return WorldSnapshot;
	}

	for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
	{
		AActor* Actor = *ActorIt;
		if (!ShouldCollectActor(Actor))
		{
			continue;
		}

		const UAtlasSavableComponent* SavableComponent = UAtlasSavableComponent::FindOnActor(Actor);
		if (!SavableComponent || !SavableComponent->HasValidActorId())
		{
			continue;
		}

		FAtlasActorSnapshot& ActorSnapshot = WorldSnapshot.AddActor(
			SavableComponent->GetActorId(),
			Actor->GetClass()->GetPathName(),
			Actor->GetActorTransform());

		CaptureActor(Actor, ActorSnapshot);
	}

	ATLAS_LOG_CORE(Log, "Collected world snapshot: actors=%d", WorldSnapshot.Actors.Num());
	return WorldSnapshot;
}

bool FAtlasSaveCollector::ShouldCollectActor(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return false;
	}

	return UAtlasSavableComponent::FindOnActor(Actor) != nullptr;
}

bool FAtlasSaveCollector::CaptureActor(AActor* Actor, FAtlasActorSnapshot& OutActorSnapshot)
{
	if (!IsValid(Actor))
	{
		return false;
	}

	bool bCapturedAnyState = false;

	if (Actor->GetClass()->ImplementsInterface(UAtlasSavable::StaticClass()))
	{
		bCapturedAnyState |= CaptureSavableObject(Actor, BuildActorChunkName(), OutActorSnapshot);
	}

	TArray<UActorComponent*> Components;
	Actor->GetComponents(Components);

	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ++ComponentIndex)
	{
		UActorComponent* Component = Components[ComponentIndex];
		if (!IsValid(Component) || !Component->GetClass()->ImplementsInterface(UAtlasSavable::StaticClass()))
		{
			continue;
		}

		bCapturedAnyState |= CaptureSavableObject(
			Component,
			BuildComponentChunkName(Component, ComponentIndex),
			OutActorSnapshot);
	}

	return bCapturedAnyState;
}

bool FAtlasSaveCollector::CaptureSavableObject(UObject* Object, FName ChunkName, FAtlasActorSnapshot& OutActorSnapshot)
{
	if (!IsValid(Object) || ChunkName.IsNone())
	{
		return false;
	}

	IAtlasSavable* SavableObject = Cast<IAtlasSavable>(Object);
	if (!SavableObject)
	{
		return false;
	}

	TArray<uint8> ChunkData;
	FMemoryWriter MemoryWriter(ChunkData, true);
	FAtlasSaveContext SaveContext(MemoryWriter);

	SaveContext.BeginChunk(ChunkName);
	SavableObject->CaptureState(SaveContext);
	SaveContext.EndChunk();

	OutActorSnapshot.AddChunk(ChunkName, MoveTemp(ChunkData));
	return true;
}

FName FAtlasSaveCollector::BuildActorChunkName()
{
	return AtlasSaveCollector::ActorChunkName;
}

FName FAtlasSaveCollector::BuildComponentChunkName(const UActorComponent* Component, int32 ComponentIndex)
{
	check(Component != nullptr);

	return FName(*FString::Printf(
		TEXT("%s.%d.%s"),
		AtlasSaveCollector::ComponentChunkPrefix,
		ComponentIndex,
		*Component->GetName()));
}
