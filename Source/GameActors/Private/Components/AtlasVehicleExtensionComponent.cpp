#include "Components/AtlasVehicleExtensionComponent.h"

#include "Components/AtlasCameraExtensionComponent.h"
#include "Components/AtlasInputExtensionComponent.h"
#include "Components/AtlasMovementExtensionComponent.h"
#include "Controller/AtlasPlayerController.h"
#include "Data/AtlasCameraConfig.h"
#include "Data/AtlasInputConfigData.h"
#include "GameFramework/Pawn.h"
#include "Logging/AtlasLogMacros.h"
#include "Pawn/AtlasVehiclePawn.h"

UAtlasVehicleExtensionComponent::UAtlasVehicleExtensionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAtlasVehicleExtensionComponent::TryEnterVehicle(APawn* Passenger, int32 SeatIndex)
{
	AAtlasVehiclePawn* Vehicle = GetOwningVehicle();
	ATLAS_ENSURE_MSG(Vehicle != nullptr, "UAtlasVehicleExtensionComponent must live on an AAtlasVehiclePawn");
	if (Vehicle == nullptr || Passenger == nullptr || !Vehicle->HasAuthority())
	{
		return false;
	}

	// Resolve the seat before touching the passenger so a full vehicle is a
	// clean no-op. Negative index means "first free seat".
	if (SeatIndex < 0)
	{
		SeatIndex = FindFreeSeat();
	}

	if (SeatIndex < 0 || SeatIndex >= SeatCount || IsSeatOccupied(SeatIndex))
	{
		ATLAS_LOG_ACTORS(Warning, "%s cannot enter %s: seat %d unavailable",
			*Passenger->GetName(), *Vehicle->GetName(), SeatIndex);
		return false;
	}

	UAtlasMovementExtensionComponent* MovementExt = UAtlasMovementExtensionComponent::FindMovementExtensionComponent(Passenger);
	if (MovementExt == nullptr)
	{
		ATLAS_LOG_ACTORS(Warning, "%s cannot enter %s: no movement extension component",
			*Passenger->GetName(), *Vehicle->GetName());
		return false;
	}

	// The movement extension performs the mechanical part (disable own
	// movement, attach to the seat, mode change).
	if (!MovementExt->EnterVehicle(Vehicle, SeatIndex))
	{
		return false;
	}

	SeatOccupants.Add(SeatIndex, Passenger);
	ApplyPassengerConfigs(Passenger);

	Vehicle->OnPassengerEntered(Passenger, SeatIndex);
	Vehicle->BP_OnPassengerEntered(Passenger, SeatIndex);
	OnPassengerEntered.Broadcast(Passenger, SeatIndex);

	ATLAS_LOG_ACTORS(Log, "%s entered %s (seat %d)", *Passenger->GetName(), *Vehicle->GetName(), SeatIndex);

	return true;
}

bool UAtlasVehicleExtensionComponent::ExitVehicle(APawn* Passenger)
{
	AAtlasVehiclePawn* Vehicle = GetOwningVehicle();
	if (Vehicle == nullptr || Passenger == nullptr || !Vehicle->HasAuthority())
	{
		return false;
	}

	const int32 SeatIndex = FindSeatOfPassenger(Passenger);
	if (SeatIndex == INDEX_NONE)
	{
		return false;
	}

	// Vehicle-specific configs come off before movement is restored so the
	// pawn never briefly has both control schemes active.
	RemovePassengerConfigs(Passenger);

	if (UAtlasMovementExtensionComponent* MovementExt = UAtlasMovementExtensionComponent::FindMovementExtensionComponent(Passenger))
	{
		MovementExt->ExitVehicle();
	}

	SeatOccupants.Remove(SeatIndex);

	Vehicle->OnPassengerExited(Passenger, SeatIndex);
	Vehicle->BP_OnPassengerExited(Passenger, SeatIndex);
	OnPassengerExited.Broadcast(Passenger, SeatIndex);

	ATLAS_LOG_ACTORS(Log, "%s exited %s (seat %d)", *Passenger->GetName(), *Vehicle->GetName(), SeatIndex);

	return true;
}

bool UAtlasVehicleExtensionComponent::IsSeatOccupied(int32 SeatIndex) const
{
	const TWeakObjectPtr<APawn>* Occupant = SeatOccupants.Find(SeatIndex);
	return Occupant != nullptr && Occupant->IsValid();
}

APawn* UAtlasVehicleExtensionComponent::GetPassengerInSeat(int32 SeatIndex) const
{
	const TWeakObjectPtr<APawn>* Occupant = SeatOccupants.Find(SeatIndex);
	return Occupant != nullptr ? Occupant->Get() : nullptr;
}

int32 UAtlasVehicleExtensionComponent::GetPassengerCount() const
{
	int32 Count = 0;
	for (const TPair<int32, TWeakObjectPtr<APawn>>& Seat : SeatOccupants)
	{
		if (Seat.Value.IsValid())
		{
			++Count;
		}
	}

	return Count;
}

int32 UAtlasVehicleExtensionComponent::GetFreeSeatCount() const
{
	return SeatCount - GetPassengerCount();
}

AAtlasVehiclePawn* UAtlasVehicleExtensionComponent::GetOwningVehicle() const
{
	return Cast<AAtlasVehiclePawn>(GetOwner());
}

int32 UAtlasVehicleExtensionComponent::FindFreeSeat() const
{
	for (int32 SeatIndex = 0; SeatIndex < SeatCount; ++SeatIndex)
	{
		if (!IsSeatOccupied(SeatIndex))
		{
			return SeatIndex;
		}
	}

	return INDEX_NONE;
}

int32 UAtlasVehicleExtensionComponent::FindSeatOfPassenger(const APawn* Passenger) const
{
	for (const TPair<int32, TWeakObjectPtr<APawn>>& Seat : SeatOccupants)
	{
		if (Seat.Value.Get() == Passenger)
		{
			return Seat.Key;
		}
	}

	return INDEX_NONE;
}

void UAtlasVehicleExtensionComponent::ApplyPassengerConfigs(APawn* Passenger)
{
	// Input: only local player passengers have an input extension to feed.
	if (const AAtlasPlayerController* PC = Cast<AAtlasPlayerController>(Passenger->GetController()))
	{
		if (const UAtlasInputConfigData* InputConfig = PassengerInputConfig.LoadSynchronous())
		{
			PC->GetInputExtensionComponent()->ApplyConfig(InputConfig);
		}
	}

	// Camera: pushed so the passenger's default config is restored on exit.
	if (UAtlasCameraExtensionComponent* CameraExt = UAtlasCameraExtensionComponent::FindCameraExtensionComponent(Passenger))
	{
		if (const UAtlasCameraConfig* CameraConfig = PassengerCameraConfig.LoadSynchronous())
		{
			CameraExt->PushCameraConfig(CameraConfig);
		}
	}
}

void UAtlasVehicleExtensionComponent::RemovePassengerConfigs(APawn* Passenger)
{
	if (const AAtlasPlayerController* PC = Cast<AAtlasPlayerController>(Passenger->GetController()))
	{
		if (const UAtlasInputConfigData* InputConfig = PassengerInputConfig.Get())
		{
			PC->GetInputExtensionComponent()->RemoveConfig(InputConfig);
		}
	}

	if (UAtlasCameraExtensionComponent* CameraExt = UAtlasCameraExtensionComponent::FindCameraExtensionComponent(Passenger))
	{
		CameraExt->ClearPushedCameraConfig();
	}
}
