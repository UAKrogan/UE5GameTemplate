#include "Actions/AtlasGameFeatureAction_AddHUDElements.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Settings/AtlasUIDeveloperSettings.h"
#include "Subsystems/AtlasUISubsystem.h"
#include "Widgets/AtlasHUDWidget.h"

void UAtlasGameFeatureAction_AddHUDElements::OnGameFeatureActivating()
{
	Super::OnGameFeatureActivating();

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		const UWorld* World = WorldContext.World();
		if (World == nullptr || !World->IsGameWorld() || World->GetGameInstance() == nullptr)
		{
			continue;
		}

		if (UAtlasUISubsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<UAtlasUISubsystem>())
		{
			// Catch HUD screens pushed after activation.
			UISubsystem->OnScreenPushed.AddDynamic(this, &UAtlasGameFeatureAction_AddHUDElements::HandleScreenPushed);
			BoundSubsystems.Add(UISubsystem);
		}
	}

	AddElementsToActiveHUDs();
}

void UAtlasGameFeatureAction_AddHUDElements::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	RemoveElementsFromActiveHUDs();

	for (const TWeakObjectPtr<UAtlasUISubsystem>& SubsystemPtr : BoundSubsystems)
	{
		if (UAtlasUISubsystem* UISubsystem = SubsystemPtr.Get())
		{
			UISubsystem->OnScreenPushed.RemoveDynamic(this, &UAtlasGameFeatureAction_AddHUDElements::HandleScreenPushed);
		}
	}

	BoundSubsystems.Empty();
}

void UAtlasGameFeatureAction_AddHUDElements::AddElementsToActiveHUDs()
{
	for (const TWeakObjectPtr<UAtlasUISubsystem>& SubsystemPtr : BoundSubsystems)
	{
		UAtlasUISubsystem* UISubsystem = SubsystemPtr.Get();
		UAtlasHUDWidget* HUDWidget = UISubsystem != nullptr ? UISubsystem->GetActiveHUDWidget() : nullptr;
		if (HUDWidget == nullptr)
		{
			continue;
		}

		for (const FAtlasHUDElementEntry& Entry : Elements)
		{
			if (HUDWidget->GetHUDElement(Entry.SlotName) != nullptr)
			{
				continue;
			}

			if (UClass* WidgetClass = Entry.WidgetClass.LoadSynchronous())
			{
				if (UUserWidget* Element = CreateWidget<UUserWidget>(HUDWidget, WidgetClass))
				{
					HUDWidget->AddHUDElement(Entry.SlotName, Element);
				}
			}
		}
	}
}

void UAtlasGameFeatureAction_AddHUDElements::RemoveElementsFromActiveHUDs()
{
	for (const TWeakObjectPtr<UAtlasUISubsystem>& SubsystemPtr : BoundSubsystems)
	{
		UAtlasUISubsystem* UISubsystem = SubsystemPtr.Get();
		UAtlasHUDWidget* HUDWidget = UISubsystem != nullptr ? UISubsystem->GetActiveHUDWidget() : nullptr;
		if (HUDWidget == nullptr)
		{
			continue;
		}

		for (const FAtlasHUDElementEntry& Entry : Elements)
		{
			HUDWidget->RemoveHUDElement(Entry.SlotName);
		}
	}
}

void UAtlasGameFeatureAction_AddHUDElements::HandleScreenPushed(FName ScreenId)
{
	if (ScreenId == UAtlasUIDeveloperSettings::Get()->HUDScreenId)
	{
		AddElementsToActiveHUDs();
	}
}
