#pragma once

#include "Camera/PlayerCameraManager.h"
#include "Engine/DataAsset.h"
#include "AtlasCameraConfig.generated.h"

/*
 * Camera and spring arm configuration for a pawn type or movement mode.
 *
 * Applied by UAtlasCameraExtensionComponent: the default config comes from
 * the pawn data; vehicles and mounts push their own config while the pawn
 * is attached to them and the default is restored on exit.
 *
 * Multiple instances per project by design (per pawn type, per mode) —
 * which is why this is a data asset and not a developer setting.
 */
UCLASS(BlueprintType)
class GAMEACTORS_API UAtlasCameraConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm")
	float SpringArmLength = 400.0f;

	/*
	 * Rest rotation of the spring arm. Only applied when the arm does not
	 * follow the pawn's control rotation.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm")
	FRotator DefaultRotation = FRotator(-15.0f, 0.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm")
	bool bUsePawnControlRotation = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm")
	bool bInheritPitch = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm")
	bool bInheritYaw = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Arm")
	bool bInheritRoll = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float FieldOfView = 90.0f;

	/*
	 * Blend timing for game code that switches view targets alongside a
	 * config change (SetViewTargetWithBlend). The camera extension itself
	 * applies configs instantly — the pawn stays the view target.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blending")
	float BlendTime = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blending")
	TEnumAsByte<EViewTargetBlendFunction> BlendFunction = VTBlend_Cubic;
};
