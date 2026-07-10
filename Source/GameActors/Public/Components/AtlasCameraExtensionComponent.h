#pragma once

#include "Components/ActorComponent.h"
#include "AtlasCameraExtensionComponent.generated.h"

class UAtlasCameraConfig;

/*
 * Optional component applying UAtlasCameraConfig data assets to the owning
 * pawn's spring arm and camera components.
 *
 * Config resolution (GetActiveCameraConfig):
 * 1. A pushed config, when set — vehicles/mounts push their config while
 *    the pawn is attached to them and clear it on exit.
 * 2. Otherwise the pawn data's CameraConfig (via the pawn extension).
 *
 * Configs are applied instantly to the existing spring arm/camera — the
 * pawn stays the view target. Games that switch view targets instead
 * (SetViewTargetWithBlend) can use the config's BlendTime/BlendFunction.
 * Pawns without a spring arm or camera simply ignore the relevant fields.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class GAMEACTORS_API UAtlasCameraExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasCameraExtensionComponent();

	static UAtlasCameraExtensionComponent* FindCameraExtensionComponent(const APawn* Pawn);

	/*
	 * Re-applies whatever GetActiveCameraConfig currently resolves to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Camera")
	void ApplyActiveCameraConfig();

	/*
	 * Temporarily overrides the default config (vehicle/mount camera) and
	 * applies it. Cleared with ClearPushedCameraConfig.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Camera")
	void PushCameraConfig(const UAtlasCameraConfig* Config);

	/*
	 * Removes the pushed override and re-applies the default config.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Camera")
	void ClearPushedCameraConfig();

	/*
	 * The config that should currently drive the camera. Override in game
	 * projects for additional layers (aim camera, photo mode, ...).
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Camera")
	virtual const UAtlasCameraConfig* GetActiveCameraConfig() const;

protected:
	//~UActorComponent interface
	virtual void BeginPlay() override;
	//~End of UActorComponent interface

private:
	/*
	 * Writes the config values into the owner's spring arm and camera
	 * components; each is optional and skipped when absent.
	 */
	void ApplyConfig(const UAtlasCameraConfig* Config);

	UPROPERTY(Transient)
	TObjectPtr<const UAtlasCameraConfig> PushedConfig;
};
