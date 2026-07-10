#include "Actions/AtlasGameFeatureAction_AddInputConfig.h"

#include "Components/AtlasInputExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Controller/AtlasPlayerController.h"
#include "Data/AtlasInputConfigData.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UAtlasGameFeatureAction_AddInputConfig::OnGameFeatureActivating()
{
	Super::OnGameFeatureActivating();

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		const UWorld* World = WorldContext.World();
		if (World == nullptr || !World->IsGameWorld() || World->GetGameInstance() == nullptr)
		{
			continue;
		}

		if (UGameFrameworkComponentManager* Manager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(World->GetGameInstance()))
		{
			ExtensionRequestHandles.Add(Manager->AddExtensionHandler(
				AAtlasPlayerController::StaticClass(),
				UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
					this, &UAtlasGameFeatureAction_AddInputConfig::HandleActorExtension)));
		}
	}
}

void UAtlasGameFeatureAction_AddInputConfig::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	for (const TWeakObjectPtr<AAtlasPlayerController>& ControllerPtr : AppliedControllers)
	{
		if (AAtlasPlayerController* Controller = ControllerPtr.Get())
		{
			RemoveFromController(Controller);
		}
	}

	AppliedControllers.Empty();
	ExtensionRequestHandles.Empty();
}

void UAtlasGameFeatureAction_AddInputConfig::HandleActorExtension(AActor* Actor, FName EventName)
{
	AAtlasPlayerController* Controller = Cast<AAtlasPlayerController>(Actor);
	if (Controller == nullptr || !Controller->IsLocalController())
	{
		return;
	}

	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded ||
		EventName == UGameFrameworkComponentManager::NAME_GameActorReady)
	{
		ApplyToController(Controller);
	}
	else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved ||
		EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
	{
		RemoveFromController(Controller);
	}
}

void UAtlasGameFeatureAction_AddInputConfig::ApplyToController(AAtlasPlayerController* Controller)
{
	UAtlasInputExtensionComponent* InputExt = Controller->GetInputExtensionComponent();
	if (InputExt == nullptr)
	{
		return;
	}

	for (const TSoftObjectPtr<UAtlasInputConfigData>& ConfigPtr : InputConfigs)
	{
		InputExt->ApplyConfig(ConfigPtr.LoadSynchronous());
	}

	AppliedControllers.Add(Controller);
}

void UAtlasGameFeatureAction_AddInputConfig::RemoveFromController(AAtlasPlayerController* Controller)
{
	UAtlasInputExtensionComponent* InputExt = Controller->GetInputExtensionComponent();
	if (InputExt == nullptr)
	{
		return;
	}

	for (const TSoftObjectPtr<UAtlasInputConfigData>& ConfigPtr : InputConfigs)
	{
		InputExt->RemoveConfig(ConfigPtr.Get());
	}

	AppliedControllers.Remove(Controller);
}
