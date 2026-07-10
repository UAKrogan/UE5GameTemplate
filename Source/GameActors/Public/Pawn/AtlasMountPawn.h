#pragma once

#include "CoreMinimal.h"
#include "Pawn/AtlasPawn.h"
#include "AtlasMountPawn.generated.h"

class UAtlasMountExtensionComponent;

/*
 * Base for rideable mounts. Supports a single rider via
 * UAtlasMountExtensionComponent.
 *
 * The mount's own movement component (added by the game project) drives
 * both the mount and the attached rider. While ridden, the mount may still
 * be AI-controlled (rider input limited to dismount) or possessed by the
 * rider's controller (game-project choice).
 */
UCLASS()
class GAMEACTORS_API AAtlasMountPawn : public AAtlasPawn
{
	GENERATED_BODY()

public:
	AAtlasMountPawn();

	/*
	 * Whether a rider may mount right now. Default: has a mount extension
	 * and no current rider. Override for taming/ownership rules.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Mount")
	virtual bool CanBeRidden() const;

	/*
	 * Scene component the rider attaches to. Default: root. Override to
	 * return the mount mesh when the rider socket lives on a skeletal mesh.
	 */
	virtual USceneComponent* GetRiderAttachComponent() const;

	/*
	 * Socket the rider snaps to. Default convention: "RiderSocket".
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Mount")
	virtual FName GetRiderAttachSocket() const;

	UAtlasMountExtensionComponent* GetMountExtensionComponent() const { return MountExtComp; }

protected:
	// The extension component calls these around each rider change so
	// subclasses can react (stop wandering AI, play mount animation, ...).
	friend class UAtlasMountExtensionComponent;

	virtual void OnRiderMounted(APawn* Rider) {}
	virtual void OnRiderDismounted(APawn* Rider) {}

	/*
	 * Blueprint mirrors of the rider hooks.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Atlas|Mount", meta = (DisplayName = "On Rider Mounted"))
	void BP_OnRiderMounted(APawn* Rider);

	UFUNCTION(BlueprintImplementableEvent, Category = "Atlas|Mount", meta = (DisplayName = "On Rider Dismounted"))
	void BP_OnRiderDismounted(APawn* Rider);

	// Mount/dismount lifecycle and rider tracking.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atlas")
	TObjectPtr<UAtlasMountExtensionComponent> MountExtComp;
};
