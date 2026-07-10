#pragma once

#include "CoreMinimal.h"
#include "Pawn/AtlasPawn.h"
#include "AtlasVehiclePawn.generated.h"

class UAtlasVehicleExtensionComponent;
class UPawnMovementComponent;

/*
 * Base for all vehicle pawns.
 *
 * Deliberately does not implement vehicle physics: game projects add their
 * preferred movement component (UChaosVehicleMovementComponent, a custom
 * component, ...) in a subclass or Blueprint. The Atlas layer provides the
 * enter/exit contract via UAtlasVehicleExtensionComponent and the seat
 * attachment points.
 *
 * Inherits the full GAS contract from AAtlasPawn — a driven vehicle can own
 * its own ASC (AI vehicle) or grant abilities to the driver's PlayerState
 * ASC (game-project work).
 */
UCLASS()
class GAMEACTORS_API AAtlasVehiclePawn : public AAtlasPawn
{
	GENERATED_BODY()

public:
	AAtlasVehiclePawn();

	/*
	 * The component that drives this vehicle. Null in the base template —
	 * resolved dynamically so subclasses/Blueprints can add any movement
	 * component without a hard dependency here.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Vehicle")
	virtual UPawnMovementComponent* GetVehicleMovementComponent() const;

	/*
	 * Scene component passengers attach to. Default: root. Override to
	 * return the vehicle mesh when seat sockets live on a skeletal mesh.
	 */
	virtual USceneComponent* GetSeatAttachComponent(int32 SeatIndex) const;

	/*
	 * Socket name for a seat. Default convention: "Seat_<Index>". Missing
	 * sockets degrade gracefully — the pawn attaches at the component origin.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Vehicle")
	virtual FName GetSeatSocketName(int32 SeatIndex) const;

	/*
	 * Number of free seats, resolved through the vehicle extension.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Vehicle")
	int32 GetAvailableSeats() const;

	UAtlasVehicleExtensionComponent* GetVehicleExtensionComponent() const { return VehicleExtComp; }

protected:
	// The extension component calls these around each passenger change so
	// subclasses can react (start engine, seat animations, ...).
	friend class UAtlasVehicleExtensionComponent;

	virtual void OnPassengerEntered(APawn* Passenger, int32 SeatIndex) {}
	virtual void OnPassengerExited(APawn* Passenger, int32 SeatIndex) {}

	/*
	 * Blueprint mirrors of the passenger hooks.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Atlas|Vehicle", meta = (DisplayName = "On Passenger Entered"))
	void BP_OnPassengerEntered(APawn* Passenger, int32 SeatIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Atlas|Vehicle", meta = (DisplayName = "On Passenger Exited"))
	void BP_OnPassengerExited(APawn* Passenger, int32 SeatIndex);

	// Enter/exit lifecycle, seat assignment, and passenger tracking.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atlas")
	TObjectPtr<UAtlasVehicleExtensionComponent> VehicleExtComp;
};
