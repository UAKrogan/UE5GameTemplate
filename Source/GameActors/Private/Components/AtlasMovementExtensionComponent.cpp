#include "Components/AtlasMovementExtensionComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Logging/AtlasLogMacros.h"
#include "Net/UnrealNetwork.h"
#include "Pawn/AtlasMountPawn.h"
#include "Pawn/AtlasVehiclePawn.h"

namespace AtlasMovement
{
	/*
	 * Maps the engine movement mode to the Atlas high-level mode. Returns
	 * None for MOVE_None so callers can distinguish "disabled" from a real
	 * locomotion state.
	 */
	EAtlasMovementMode MapEngineMode(EMovementMode EngineMode)
	{
		switch (EngineMode)
		{
		case MOVE_Walking:
		case MOVE_NavWalking:
			return EAtlasMovementMode::Walking;
		case MOVE_Falling:
			return EAtlasMovementMode::Falling;
		case MOVE_Swimming:
			return EAtlasMovementMode::Swimming;
		case MOVE_Flying:
			return EAtlasMovementMode::Flying;
		case MOVE_Custom:
			return EAtlasMovementMode::Custom;
		default:
			return EAtlasMovementMode::None;
		}
	}
}

UAtlasMovementExtensionComponent::UAtlasMovementExtensionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// The mode itself replicates; attachment replicates with the pawn.
	SetIsReplicatedByDefault(true);
}

UAtlasMovementExtensionComponent* UAtlasMovementExtensionComponent::FindMovementExtensionComponent(const APawn* Pawn)
{
	return Pawn != nullptr ? Pawn->FindComponentByClass<UAtlasMovementExtensionComponent>() : nullptr;
}

void UAtlasMovementExtensionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAtlasMovementExtensionComponent, CurrentMode);
}

void UAtlasMovementExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Character owners: mirror engine locomotion mode changes so gameplay
	// listening to the Atlas mode never needs the movement component.
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->MovementModeChangedDelegate.AddDynamic(
			this, &UAtlasMovementExtensionComponent::HandleCharacterMovementModeChanged);

		if (const UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
		{
			const EAtlasMovementMode InitialMode = AtlasMovement::MapEngineMode(CharacterMovement->MovementMode);
			if (InitialMode != EAtlasMovementMode::None)
			{
				CurrentMode = InitialMode;
			}
		}
	}
}

void UAtlasMovementExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->MovementModeChangedDelegate.RemoveDynamic(
			this, &UAtlasMovementExtensionComponent::HandleCharacterMovementModeChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void UAtlasMovementExtensionComponent::RequestMovementModeChange(EAtlasMovementMode NewMode)
{
	// Attachment states carry side effects (movement disable, attach) that
	// only the dedicated transitions perform correctly.
	if (NewMode == EAtlasMovementMode::InVehicle || NewMode == EAtlasMovementMode::Mounted)
	{
		ATLAS_ENSURE_MSG(false, "Use EnterVehicle/MountRider for attachment modes, not RequestMovementModeChange");
		return;
	}

	if (IsAttachedMode())
	{
		ATLAS_LOG_ACTORS(Warning, "Movement mode change to %d ignored while attached (mode %d)",
			static_cast<int32>(NewMode), static_cast<int32>(CurrentMode));
		return;
	}

	SetMovementMode(NewMode);
}

bool UAtlasMovementExtensionComponent::EnterVehicle(AAtlasVehiclePawn* Vehicle, int32 SeatIndex)
{
	APawn* Pawn = GetOwningPawn();
	if (!ATLAS_ENSURE(Vehicle != nullptr) || Pawn == nullptr || !HasAuthority())
	{
		return false;
	}

	if (IsAttachedMode())
	{
		ATLAS_LOG_ACTORS(Warning, "%s cannot enter %s: already in vehicle/mounted", *Pawn->GetName(), *Vehicle->GetName());
		return false;
	}

	// Order matters: save state, stop self-propulsion, then attach —
	// attaching a pawn with an active movement component fights the attach.
	SavedModeBeforeAttachment = CurrentMode;
	SetOwnerMovementEnabled(false);

	// Collision between the pawn and the vehicle (capsule overlap, ragdoll
	// on exit, etc.) is a game-project concern; the template only attaches.
	Pawn->AttachToComponent(Vehicle->GetSeatAttachComponent(SeatIndex),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		Vehicle->GetSeatSocketName(SeatIndex));

	CurrentVehicle = Vehicle;
	CurrentSeatIndex = SeatIndex;
	SetMovementMode(EAtlasMovementMode::InVehicle);

	return true;
}

bool UAtlasMovementExtensionComponent::ExitVehicle()
{
	APawn* Pawn = GetOwningPawn();
	if (Pawn == nullptr || !HasAuthority() || CurrentMode != EAtlasMovementMode::InVehicle)
	{
		return false;
	}

	// Where the pawn ends up (exit point selection, safe-spot traces) is a
	// game-project concern; the template detaches in place.
	Pawn->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetOwnerMovementEnabled(true);

	CurrentVehicle.Reset();
	CurrentSeatIndex = INDEX_NONE;
	SetMovementMode(SavedModeBeforeAttachment);

	return true;
}

bool UAtlasMovementExtensionComponent::MountRider(AAtlasMountPawn* Mount)
{
	APawn* Pawn = GetOwningPawn();
	if (!ATLAS_ENSURE(Mount != nullptr) || Pawn == nullptr || !HasAuthority())
	{
		return false;
	}

	if (IsAttachedMode())
	{
		ATLAS_LOG_ACTORS(Warning, "%s cannot mount %s: already in vehicle/mounted", *Pawn->GetName(), *Mount->GetName());
		return false;
	}

	SavedModeBeforeAttachment = CurrentMode;
	SetOwnerMovementEnabled(false);

	Pawn->AttachToComponent(Mount->GetRiderAttachComponent(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		Mount->GetRiderAttachSocket());

	CurrentMount = Mount;
	SetMovementMode(EAtlasMovementMode::Mounted);

	return true;
}

bool UAtlasMovementExtensionComponent::Dismount()
{
	APawn* Pawn = GetOwningPawn();
	if (Pawn == nullptr || !HasAuthority() || CurrentMode != EAtlasMovementMode::Mounted)
	{
		return false;
	}

	Pawn->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetOwnerMovementEnabled(true);

	CurrentMount.Reset();
	SetMovementMode(SavedModeBeforeAttachment);

	return true;
}

void UAtlasMovementExtensionComponent::SetMovementMode(EAtlasMovementMode NewMode)
{
	if (NewMode == CurrentMode)
	{
		return;
	}

	const EAtlasMovementMode OldMode = CurrentMode;
	CurrentMode = NewMode;

	OnMovementModeChanged.Broadcast(OldMode, NewMode);

	ATLAS_LOG_ACTORS(Log, "%s movement mode: %d -> %d",
		*GetNameSafe(GetOwner()), static_cast<int32>(OldMode), static_cast<int32>(NewMode));
}

void UAtlasMovementExtensionComponent::SetOwnerMovementEnabled(bool bEnabled)
{
	const APawn* Pawn = GetOwningPawn();
	UPawnMovementComponent* MovementComponent = Pawn != nullptr ? Pawn->GetMovementComponent() : nullptr;
	if (MovementComponent == nullptr)
	{
		return;
	}

	if (UCharacterMovementComponent* CharacterMovement = Cast<UCharacterMovementComponent>(MovementComponent))
	{
		if (bEnabled)
		{
			// Let the movement component pick walking/falling based on the
			// current floor rather than forcing a specific mode.
			CharacterMovement->SetDefaultMovementMode();
		}
		else
		{
			CharacterMovement->StopMovementImmediately();
			CharacterMovement->SetMovementMode(MOVE_None);
		}
		return;
	}

	// Generic pawn movement (e.g. UFloatingPawnMovement): deactivating stops
	// it from consuming input and moving the pawn while attached.
	if (bEnabled)
	{
		MovementComponent->Activate();
	}
	else
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->Deactivate();
	}
}

void UAtlasMovementExtensionComponent::HandleCharacterMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	// While attached we force MOVE_None ourselves; ignore the engine echo so
	// InVehicle/Mounted are not overwritten.
	if (IsAttachedMode())
	{
		return;
	}

	const UCharacterMovementComponent* CharacterMovement = Character != nullptr ? Character->GetCharacterMovement() : nullptr;
	if (CharacterMovement == nullptr)
	{
		return;
	}

	const EAtlasMovementMode NewMode = AtlasMovement::MapEngineMode(CharacterMovement->MovementMode);
	if (NewMode != EAtlasMovementMode::None)
	{
		SetMovementMode(NewMode);
	}
}

void UAtlasMovementExtensionComponent::OnRep_MovementMode(EAtlasMovementMode OldMode)
{
	// Clients rebroadcast so local listeners (input suppression, animation,
	// UI) react without any custom replication of their own.
	OnMovementModeChanged.Broadcast(OldMode, CurrentMode);
}

APawn* UAtlasMovementExtensionComponent::GetOwningPawn() const
{
	return Cast<APawn>(GetOwner());
}

bool UAtlasMovementExtensionComponent::HasAuthority() const
{
	const AActor* Owner = GetOwner();
	return Owner != nullptr && Owner->HasAuthority();
}
