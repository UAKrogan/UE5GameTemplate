#pragma once

#include "Components/ActorComponent.h"
#include "AtlasVehicleExtensionComponent.generated.h"

class AAtlasVehiclePawn;
class UAtlasCameraConfig;
class UAtlasInputConfigData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAtlasVehiclePassengerEvent, APawn*, Passenger, int32, SeatIndex);

/*
 * Handles the enter/exit lifecycle for a vehicle pawn: seat assignment,
 * passenger tracking, and passenger-side input/camera configuration.
 *
 * Enter flow (authority):
 * 1. TryEnterVehicle validates the passenger and resolves a free seat.
 * 2. The passenger's movement extension disables its own movement and
 *    attaches the pawn to the seat (which also suppresses the passenger's
 *    default input mapping contexts via the input extension).
 * 3. Optional PassengerInputConfig / PassengerCameraConfig are applied to
 *    the passenger (vehicle controls, vehicle camera).
 * 4. The vehicle's OnPassengerEntered hooks and the delegate fire.
 *
 * Exit reverses the same steps. Must live on an AAtlasVehiclePawn.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class GAMEACTORS_API UAtlasVehicleExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasVehicleExtensionComponent();

	/*
	 * Seats the passenger. SeatIndex -1 picks the first free seat. Returns
	 * false when full, occupied, off-authority, or the passenger has no
	 * movement extension component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Vehicle")
	bool TryEnterVehicle(APawn* Passenger, int32 SeatIndex = -1);

	/*
	 * Removes the passenger from its seat and restores its own movement and
	 * input. Returns false when the pawn is not a passenger of this vehicle.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Vehicle")
	bool ExitVehicle(APawn* Passenger);

	UFUNCTION(BlueprintCallable, Category = "Atlas|Vehicle")
	bool IsSeatOccupied(int32 SeatIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Atlas|Vehicle")
	APawn* GetPassengerInSeat(int32 SeatIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Atlas|Vehicle")
	int32 GetPassengerCount() const;

	UFUNCTION(BlueprintCallable, Category = "Atlas|Vehicle")
	int32 GetFreeSeatCount() const;

	UPROPERTY(BlueprintAssignable, Category = "Atlas|Vehicle")
	FAtlasVehiclePassengerEvent OnPassengerEntered;

	UPROPERTY(BlueprintAssignable, Category = "Atlas|Vehicle")
	FAtlasVehiclePassengerEvent OnPassengerExited;

protected:
	/*
	 * Total seats. Seat indices run 0..SeatCount-1; by convention seat 0 is
	 * the driver.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atlas|Vehicle", meta = (ClampMin = 1))
	int32 SeatCount = 1;

	/*
	 * Input config applied to a player passenger while seated (drive
	 * controls, exit action). Optional; the passenger's default pawn
	 * contexts are already suppressed by the movement mode change.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atlas|Vehicle")
	TSoftObjectPtr<UAtlasInputConfigData> PassengerInputConfig;

	/*
	 * Camera config pushed onto the passenger's camera extension while
	 * seated (e.g. chase camera). Optional.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atlas|Vehicle")
	TSoftObjectPtr<UAtlasCameraConfig> PassengerCameraConfig;

private:
	AAtlasVehiclePawn* GetOwningVehicle() const;
	int32 FindFreeSeat() const;
	int32 FindSeatOfPassenger(const APawn* Passenger) const;
	void ApplyPassengerConfigs(APawn* Passenger);
	void RemovePassengerConfigs(APawn* Passenger);

	// Seat index -> current occupant. Weak so a destroyed passenger never
	// leaves a dangling seat; stale entries read back as free.
	TMap<int32, TWeakObjectPtr<APawn>> SeatOccupants;
};
