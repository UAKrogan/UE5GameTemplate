#pragma once

#include "Components/ActorComponent.h"
#include "AtlasMountExtensionComponent.generated.h"

class AAtlasMountPawn;
class UAtlasCameraConfig;
class UAtlasInputConfigData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAtlasMountRiderEvent, APawn*, Rider);

/*
 * Handles the mount/dismount lifecycle for a mount pawn: single-rider
 * tracking, rider attachment (via the rider's movement extension), and
 * rider-side input/camera configuration.
 *
 * Mirrors UAtlasVehicleExtensionComponent with a single implicit "seat".
 * Must live on an AAtlasMountPawn.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class GAMEACTORS_API UAtlasMountExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasMountExtensionComponent();

	/*
	 * Attaches the rider to the mount's rider socket. Returns false when
	 * the mount refuses (CanBeRidden), is already ridden, off-authority, or
	 * the rider has no movement extension component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Mount")
	bool TryMount(APawn* Rider);

	/*
	 * Detaches the current rider and restores its own movement and input.
	 * Returns false when nothing is mounted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Mount")
	bool Dismount();

	UFUNCTION(BlueprintCallable, Category = "Atlas|Mount")
	bool IsMounted() const { return CurrentRider.IsValid(); }

	UFUNCTION(BlueprintCallable, Category = "Atlas|Mount")
	APawn* GetCurrentRider() const { return CurrentRider.Get(); }

	UPROPERTY(BlueprintAssignable, Category = "Atlas|Mount")
	FAtlasMountRiderEvent OnRiderMounted;

	UPROPERTY(BlueprintAssignable, Category = "Atlas|Mount")
	FAtlasMountRiderEvent OnRiderDismounted;

protected:
	/*
	 * Input config applied to a player rider while mounted (steer the
	 * mount, dismount action). Optional; the rider's default pawn contexts
	 * are already suppressed by the movement mode change.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atlas|Mount")
	TSoftObjectPtr<UAtlasInputConfigData> RiderInputConfig;

	/*
	 * Camera config pushed onto the rider's camera extension while mounted.
	 * Optional.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atlas|Mount")
	TSoftObjectPtr<UAtlasCameraConfig> RiderCameraConfig;

private:
	AAtlasMountPawn* GetOwningMount() const;
	void ApplyRiderConfigs(APawn* Rider);
	void RemoveRiderConfigs(APawn* Rider);

	// Weak so a destroyed rider never leaves the mount permanently "ridden".
	TWeakObjectPtr<APawn> CurrentRider;
};
