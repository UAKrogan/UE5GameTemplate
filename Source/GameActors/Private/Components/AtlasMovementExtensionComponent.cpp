#include "Components/AtlasMovementExtensionComponent.h"

#include "GameFramework/Pawn.h"
#include "Logging/AtlasLogMacros.h"

UAtlasMovementExtensionComponent::UAtlasMovementExtensionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UAtlasMovementExtensionComponent* UAtlasMovementExtensionComponent::FindMovementExtensionComponent(const APawn* Pawn)
{
	return Pawn != nullptr ? Pawn->FindComponentByClass<UAtlasMovementExtensionComponent>() : nullptr;
}

void UAtlasMovementExtensionComponent::RequestMovementModeChange(EAtlasMovementMode NewMode)
{
	if (NewMode == CurrentMode)
	{
		return;
	}

	const EAtlasMovementMode OldMode = CurrentMode;
	CurrentMode = NewMode;

	OnMovementModeChanged.Broadcast(OldMode, NewMode);

	ATLAS_LOG_ACTORS(Log, "Movement mode: %d -> %d", static_cast<int32>(OldMode), static_cast<int32>(NewMode));
}
