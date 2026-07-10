#include "Components/AtlasMountExtensionComponent.h"

#include "Components/AtlasCameraExtensionComponent.h"
#include "Components/AtlasInputExtensionComponent.h"
#include "Components/AtlasMovementExtensionComponent.h"
#include "Controller/AtlasPlayerController.h"
#include "Data/AtlasCameraConfig.h"
#include "Data/AtlasInputConfigData.h"
#include "GameFramework/Pawn.h"
#include "Logging/AtlasLogMacros.h"
#include "Pawn/AtlasMountPawn.h"

UAtlasMountExtensionComponent::UAtlasMountExtensionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAtlasMountExtensionComponent::TryMount(APawn* Rider)
{
	AAtlasMountPawn* Mount = GetOwningMount();
	ATLAS_ENSURE_MSG(Mount != nullptr, "UAtlasMountExtensionComponent must live on an AAtlasMountPawn");
	if (Mount == nullptr || Rider == nullptr || !Mount->HasAuthority())
	{
		return false;
	}

	if (IsMounted() || !Mount->CanBeRidden())
	{
		ATLAS_LOG_ACTORS(Warning, "%s cannot mount %s: mount refused or occupied",
			*Rider->GetName(), *Mount->GetName());
		return false;
	}

	UAtlasMovementExtensionComponent* MovementExt = UAtlasMovementExtensionComponent::FindMovementExtensionComponent(Rider);
	if (MovementExt == nullptr)
	{
		ATLAS_LOG_ACTORS(Warning, "%s cannot mount %s: no movement extension component",
			*Rider->GetName(), *Mount->GetName());
		return false;
	}

	// The movement extension performs the mechanical part (disable own
	// movement, attach to the rider socket, mode change).
	if (!MovementExt->MountRider(Mount))
	{
		return false;
	}

	CurrentRider = Rider;
	ApplyRiderConfigs(Rider);

	Mount->OnRiderMounted(Rider);
	Mount->BP_OnRiderMounted(Rider);
	OnRiderMounted.Broadcast(Rider);

	ATLAS_LOG_ACTORS(Log, "%s mounted %s", *Rider->GetName(), *Mount->GetName());

	return true;
}

bool UAtlasMountExtensionComponent::Dismount()
{
	AAtlasMountPawn* Mount = GetOwningMount();
	APawn* Rider = CurrentRider.Get();
	if (Mount == nullptr || Rider == nullptr || !Mount->HasAuthority())
	{
		return false;
	}

	// Mount-specific configs come off before movement is restored so the
	// rider never briefly has both control schemes active.
	RemoveRiderConfigs(Rider);

	if (UAtlasMovementExtensionComponent* MovementExt = UAtlasMovementExtensionComponent::FindMovementExtensionComponent(Rider))
	{
		MovementExt->Dismount();
	}

	CurrentRider.Reset();

	Mount->OnRiderDismounted(Rider);
	Mount->BP_OnRiderDismounted(Rider);
	OnRiderDismounted.Broadcast(Rider);

	ATLAS_LOG_ACTORS(Log, "%s dismounted %s", *Rider->GetName(), *Mount->GetName());

	return true;
}

AAtlasMountPawn* UAtlasMountExtensionComponent::GetOwningMount() const
{
	return Cast<AAtlasMountPawn>(GetOwner());
}

void UAtlasMountExtensionComponent::ApplyRiderConfigs(APawn* Rider)
{
	// Input: only local player riders have an input extension to feed.
	if (const AAtlasPlayerController* PC = Cast<AAtlasPlayerController>(Rider->GetController()))
	{
		if (const UAtlasInputConfigData* InputConfig = RiderInputConfig.LoadSynchronous())
		{
			PC->GetInputExtensionComponent()->ApplyConfig(InputConfig);
		}
	}

	// Camera: pushed so the rider's default config is restored on dismount.
	if (UAtlasCameraExtensionComponent* CameraExt = UAtlasCameraExtensionComponent::FindCameraExtensionComponent(Rider))
	{
		if (const UAtlasCameraConfig* CameraConfig = RiderCameraConfig.LoadSynchronous())
		{
			CameraExt->PushCameraConfig(CameraConfig);
		}
	}
}

void UAtlasMountExtensionComponent::RemoveRiderConfigs(APawn* Rider)
{
	if (const AAtlasPlayerController* PC = Cast<AAtlasPlayerController>(Rider->GetController()))
	{
		if (const UAtlasInputConfigData* InputConfig = RiderInputConfig.Get())
		{
			PC->GetInputExtensionComponent()->RemoveConfig(InputConfig);
		}
	}

	if (UAtlasCameraExtensionComponent* CameraExt = UAtlasCameraExtensionComponent::FindCameraExtensionComponent(Rider))
	{
		CameraExt->ClearPushedCameraConfig();
	}
}
