#include "Pawn/AtlasMountPawn.h"

#include "Components/AtlasMountExtensionComponent.h"

AAtlasMountPawn::AAtlasMountPawn()
{
	MountExtComp = CreateDefaultSubobject<UAtlasMountExtensionComponent>(TEXT("MountExtComp"));
}

bool AAtlasMountPawn::CanBeRidden() const
{
	return MountExtComp != nullptr && !MountExtComp->IsMounted();
}

USceneComponent* AAtlasMountPawn::GetRiderAttachComponent() const
{
	return GetRootComponent();
}

FName AAtlasMountPawn::GetRiderAttachSocket() const
{
	return TEXT("RiderSocket");
}
