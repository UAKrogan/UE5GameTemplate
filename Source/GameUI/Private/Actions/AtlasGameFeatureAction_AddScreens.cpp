#include "Actions/AtlasGameFeatureAction_AddScreens.h"

#include "Data/AtlasScreenDefinition.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Subsystems/AtlasUISubsystem.h"

namespace
{
	template <typename FuncT>
	void ForEachUISubsystem(FuncT Func)
	{
		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			const UWorld* World = WorldContext.World();
			if (World == nullptr || !World->IsGameWorld() || World->GetGameInstance() == nullptr)
			{
				continue;
			}

			if (UAtlasUISubsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<UAtlasUISubsystem>())
			{
				Func(UISubsystem);
			}
		}
	}
}

void UAtlasGameFeatureAction_AddScreens::OnGameFeatureActivating()
{
	Super::OnGameFeatureActivating();

	ForEachUISubsystem([this](UAtlasUISubsystem* UISubsystem)
	{
		for (const FAtlasScreenRegistration& Registration : Screens)
		{
			if (UAtlasScreenDefinition* Definition = Registration.Definition.LoadSynchronous())
			{
				UISubsystem->RegisterScreen(Registration.ScreenId, Definition);
			}
		}
	});
}

void UAtlasGameFeatureAction_AddScreens::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	ForEachUISubsystem([this](UAtlasUISubsystem* UISubsystem)
	{
		for (const FAtlasScreenRegistration& Registration : Screens)
		{
			UISubsystem->UnregisterScreen(Registration.ScreenId);
		}
	});
}
