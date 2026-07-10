#include "Pawn/AtlasVehiclePawn.h"

#include "Components/AtlasVehicleExtensionComponent.h"
#include "GameFramework/PawnMovementComponent.h"

AAtlasVehiclePawn::AAtlasVehiclePawn()
{
	VehicleExtComp = CreateDefaultSubobject<UAtlasVehicleExtensionComponent>(TEXT("VehicleExtComp"));
}

UPawnMovementComponent* AAtlasVehiclePawn::GetVehicleMovementComponent() const
{
	// Whatever movement component the game project added; the base template
	// ships none (vehicle physics is out of framework scope).
	return GetMovementComponent();
}

USceneComponent* AAtlasVehiclePawn::GetSeatAttachComponent(int32 SeatIndex) const
{
	return GetRootComponent();
}

FName AAtlasVehiclePawn::GetSeatSocketName(int32 SeatIndex) const
{
	return FName(*FString::Printf(TEXT("Seat_%d"), SeatIndex));
}

int32 AAtlasVehiclePawn::GetAvailableSeats() const
{
	return VehicleExtComp != nullptr ? VehicleExtComp->GetFreeSeatCount() : 0;
}
