#include "Components/AtlasCameraExtensionComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/AtlasPawnExtensionComponent.h"
#include "Data/AtlasCameraConfig.h"
#include "Data/AtlasPawnData.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Logging/AtlasLogMacros.h"

UAtlasCameraExtensionComponent::UAtlasCameraExtensionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UAtlasCameraExtensionComponent* UAtlasCameraExtensionComponent::FindCameraExtensionComponent(const APawn* Pawn)
{
	return Pawn != nullptr ? Pawn->FindComponentByClass<UAtlasCameraExtensionComponent>() : nullptr;
}

void UAtlasCameraExtensionComponent::ApplyActiveCameraConfig()
{
	ApplyConfig(GetActiveCameraConfig());
}

void UAtlasCameraExtensionComponent::PushCameraConfig(const UAtlasCameraConfig* Config)
{
	if (Config == nullptr)
	{
		return;
	}

	PushedConfig = Config;
	ApplyConfig(Config);
}

void UAtlasCameraExtensionComponent::ClearPushedCameraConfig()
{
	if (PushedConfig == nullptr)
	{
		return;
	}

	PushedConfig = nullptr;
	ApplyActiveCameraConfig();
}

const UAtlasCameraConfig* UAtlasCameraExtensionComponent::GetActiveCameraConfig() const
{
	if (PushedConfig != nullptr)
	{
		return PushedConfig;
	}

	// Default: the pawn data's camera config, resolved through the pawn
	// extension so this component needs no configuration of its own.
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const UAtlasPawnExtensionComponent* PawnExt = UAtlasPawnExtensionComponent::FindPawnExtensionComponent(Pawn);
	const UAtlasPawnData* PawnData = PawnExt != nullptr ? PawnExt->GetPawnData() : nullptr;

	return PawnData != nullptr ? PawnData->CameraConfig.LoadSynchronous() : nullptr;
}

void UAtlasCameraExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Pawn data is available by now for placed/spawned pawns; possession-
	// time data changes can call ApplyActiveCameraConfig explicitly.
	ApplyActiveCameraConfig();
}

void UAtlasCameraExtensionComponent::ApplyConfig(const UAtlasCameraConfig* Config)
{
	if (Config == nullptr)
	{
		return;
	}

	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	if (USpringArmComponent* SpringArm = Owner->FindComponentByClass<USpringArmComponent>())
	{
		SpringArm->TargetArmLength = Config->SpringArmLength;
		SpringArm->bUsePawnControlRotation = Config->bUsePawnControlRotation;
		SpringArm->bInheritPitch = Config->bInheritPitch;
		SpringArm->bInheritYaw = Config->bInheritYaw;
		SpringArm->bInheritRoll = Config->bInheritRoll;

		// A control-rotation-driven arm would immediately override a fixed
		// rest rotation, so only apply it for detached arms.
		if (!Config->bUsePawnControlRotation)
		{
			SpringArm->SetRelativeRotation(Config->DefaultRotation);
		}
	}

	if (UCameraComponent* Camera = Owner->FindComponentByClass<UCameraComponent>())
	{
		Camera->SetFieldOfView(Config->FieldOfView);
	}

	ATLAS_LOG_ACTORS(Verbose, "Camera config applied to %s: %s", *Owner->GetName(), *Config->GetName());
}
