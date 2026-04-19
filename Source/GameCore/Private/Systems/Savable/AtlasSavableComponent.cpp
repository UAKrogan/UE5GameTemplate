#include "Systems/Savable/AtlasSavableComponent.h"

#include "GameFramework/Actor.h"

UAtlasSavableComponent::UAtlasSavableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
}

void UAtlasSavableComponent::OnRegister()
{
	Super::OnRegister();

	EnsureActorId();
}

void UAtlasSavableComponent::PostLoad()
{
	Super::PostLoad();

	EnsureActorId();
}

void UAtlasSavableComponent::PostDuplicate(EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);

	if (!IsTemplate())
	{
		GenerateActorId();
	}
}

const FGuid& UAtlasSavableComponent::GetActorId() const
{
	return ActorId;
}

bool UAtlasSavableComponent::HasValidActorId() const
{
	return ActorId.IsValid();
}

void UAtlasSavableComponent::EnsureActorId()
{
	if (IsTemplate() || ActorId.IsValid())
	{
		return;
	}

	GenerateActorId();
}

void UAtlasSavableComponent::SetActorId(const FGuid& InActorId)
{
	if (InActorId.IsValid())
	{
		ActorId = InActorId;
	}
}

UAtlasSavableComponent* UAtlasSavableComponent::FindOnActor(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UAtlasSavableComponent>() : nullptr;
}

void UAtlasSavableComponent::GenerateActorId()
{
	ActorId = FGuid::NewGuid();
}
