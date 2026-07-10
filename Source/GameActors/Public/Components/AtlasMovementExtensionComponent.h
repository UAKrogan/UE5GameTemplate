#pragma once

#include "Components/ActorComponent.h"
#include "AtlasMovementExtensionComponent.generated.h"

class AAtlasMountPawn;
class AAtlasVehiclePawn;
class ACharacter;

/*
 * Authoritative high-level movement mode of a pawn.
 *
 * Walking/Falling/Swimming/Flying/Custom mirror the engine's EMovementMode
 * (kept in sync automatically for ACharacter owners). InVehicle and Mounted
 * are Atlas-level attachment states in which the pawn's own movement
 * component is disabled and something else (vehicle, mount) moves it.
 */
UENUM(BlueprintType)
enum class EAtlasMovementMode : uint8
{
	// No movement (also the transient state while the pawn is disabled).
	None,
	Walking,
	Falling,
	Swimming,
	Flying,
	// Game-specific engine custom mode (climbing, grappling, ...).
	Custom,
	// Passenger or driver of a vehicle; own movement component disabled.
	InVehicle,
	// Riding a mount; the mount's movement drives both.
	Mounted
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAtlasMovementModeChanged, EAtlasMovementMode, OldMode, EAtlasMovementMode, NewMode);

/*
 * Tracks the authoritative movement mode, routes vehicle/mount transitions,
 * and broadcasts change events. Does not directly drive physics — the
 * engine movement components stay responsible for actual motion.
 *
 * Responsibilities per transition (EnterVehicle / MountRider):
 * 1. Save the current mode for restoration on exit.
 * 2. Disable the pawn's own movement component.
 * 3. Attach the pawn to the vehicle seat / mount rider socket.
 * 4. Broadcast OnMovementModeChanged (the input extension component listens
 *    and suppresses the pawn's default mapping contexts while attached).
 *
 * Networking: the mode replicates (rep-notify rebroadcasts on clients);
 * transitions must be initiated on the authority. Attachment itself is
 * replicated natively by the engine when the pawn replicates movement.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class GAMEACTORS_API UAtlasMovementExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasMovementExtensionComponent();

	static UAtlasMovementExtensionComponent* FindMovementExtensionComponent(const APawn* Pawn);

	//~UActorComponent interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UActorComponent interface

	UFUNCTION(BlueprintCallable, Category = "Atlas|Movement")
	EAtlasMovementMode GetCurrentMovementMode() const { return CurrentMode; }

	/*
	 * Generic mode change for self-propelled modes (swim, fly, custom).
	 * InVehicle/Mounted are rejected here — those transitions carry
	 * attachment side effects and must go through EnterVehicle/MountRider.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Movement")
	void RequestMovementModeChange(EAtlasMovementMode NewMode);

	// ── Vehicle transitions ──────────────────────────────────────────
	// Called by UAtlasVehicleExtensionComponent, which owns seat
	// bookkeeping; do not call directly unless bypassing seat management.

	/*
	 * Disables own movement, attaches the pawn to the vehicle's seat
	 * socket, and enters InVehicle. Authority only.
	 */
	bool EnterVehicle(AAtlasVehiclePawn* Vehicle, int32 SeatIndex);

	/*
	 * Detaches from the current vehicle, re-enables own movement, and
	 * restores the mode saved on entry. Authority only.
	 */
	bool ExitVehicle();

	// ── Mount transitions ────────────────────────────────────────────
	// Called by UAtlasMountExtensionComponent, which owns rider
	// bookkeeping; same authority rules as the vehicle pair.

	/*
	 * Disables own movement, attaches the pawn (the rider) to the mount's
	 * rider socket, and enters Mounted. Authority only.
	 */
	bool MountRider(AAtlasMountPawn* Mount);

	/*
	 * Detaches from the current mount, re-enables own movement, and
	 * restores the mode saved on mounting. Authority only.
	 */
	bool Dismount();

	UFUNCTION(BlueprintCallable, Category = "Atlas|Movement")
	AAtlasVehiclePawn* GetCurrentVehicle() const { return CurrentVehicle.Get(); }

	UFUNCTION(BlueprintCallable, Category = "Atlas|Movement")
	AAtlasMountPawn* GetCurrentMount() const { return CurrentMount.Get(); }

	UFUNCTION(BlueprintCallable, Category = "Atlas|Movement")
	int32 GetCurrentSeatIndex() const { return CurrentSeatIndex; }

	UPROPERTY(BlueprintAssignable, Category = "Atlas|Movement")
	FAtlasMovementModeChanged OnMovementModeChanged;

protected:
	//~UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

private:
	/*
	 * Central mode setter: updates state and broadcasts. All transitions
	 * funnel through here so listeners see every change exactly once.
	 */
	void SetMovementMode(EAtlasMovementMode NewMode);

	/*
	 * Enables/disables the pawn's own movement component. Characters get
	 * MOVE_None / SetDefaultMovementMode; generic pawn movement components
	 * are deactivated/reactivated.
	 */
	void SetOwnerMovementEnabled(bool bEnabled);

	/*
	 * Mirrors engine movement mode changes (walk/fall/swim/fly/custom) into
	 * the Atlas mode while the pawn is self-propelled. Bound only for
	 * ACharacter owners.
	 */
	UFUNCTION()
	void HandleCharacterMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);

	UFUNCTION()
	void OnRep_MovementMode(EAtlasMovementMode OldMode);

	bool IsAttachedMode() const
	{
		return CurrentMode == EAtlasMovementMode::InVehicle || CurrentMode == EAtlasMovementMode::Mounted;
	}

	APawn* GetOwningPawn() const;
	bool HasAuthority() const;

	// Replicated so clients can update visual/input state; transitions are
	// authority-driven and the value only changes through SetMovementMode.
	UPROPERTY(Transient, ReplicatedUsing = OnRep_MovementMode)
	EAtlasMovementMode CurrentMode = EAtlasMovementMode::Walking;

	// Mode to restore when leaving a vehicle/mount. Authority only.
	EAtlasMovementMode SavedModeBeforeAttachment = EAtlasMovementMode::Walking;

	TWeakObjectPtr<AAtlasVehiclePawn> CurrentVehicle;
	TWeakObjectPtr<AAtlasMountPawn> CurrentMount;
	int32 CurrentSeatIndex = INDEX_NONE;
};
