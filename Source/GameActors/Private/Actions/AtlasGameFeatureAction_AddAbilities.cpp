#include "Actions/AtlasGameFeatureAction_AddAbilities.h"

#include "Components/AtlasPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GAS/AtlasAbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "Logging/AtlasLogMacros.h"

void UAtlasGameFeatureAction_AddAbilities::OnGameFeatureActivating()
{
	Super::OnGameFeatureActivating();

	UClass* TargetClass = ActorClass.LoadSynchronous();
	if (TargetClass == nullptr)
	{
		ATLAS_LOG_ACTORS(Warning, "AddAbilities action has no actor class configured");
		return;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		const UWorld* World = WorldContext.World();
		if (World == nullptr || !World->IsGameWorld() || World->GetGameInstance() == nullptr)
		{
			continue;
		}

		if (UGameFrameworkComponentManager* Manager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(World->GetGameInstance()))
		{
			// Covers live actors immediately and late-spawning actors later.
			ExtensionRequestHandles.Add(Manager->AddExtensionHandler(
				TargetClass,
				UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
					this, &UAtlasGameFeatureAction_AddAbilities::HandleActorExtension)));
		}
	}
}

void UAtlasGameFeatureAction_AddAbilities::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	Reset();
}

void UAtlasGameFeatureAction_AddAbilities::HandleActorExtension(AActor* Actor, FName EventName)
{
	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded ||
		EventName == UGameFrameworkComponentManager::NAME_GameActorReady)
	{
		TryGrantForActor(Actor);
	}
	else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved ||
		EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
	{
		RevokeForActor(Actor);
	}
}

void UAtlasGameFeatureAction_AddAbilities::TryGrantForActor(AActor* Actor)
{
	if (Actor == nullptr || !Actor->HasAuthority() || GrantedHandles.Contains(Actor))
	{
		return;
	}

	UAtlasPawnExtensionComponent* PawnExt = UAtlasPawnExtensionComponent::FindPawnExtensionComponent(Cast<APawn>(Actor));
	if (PawnExt == nullptr)
	{
		return;
	}

	UAtlasAbilitySystemComponent* ASC = PawnExt->GetAtlasAbilitySystemComponent();
	if (ASC == nullptr)
	{
		// Not possessed yet: retry when the ability system initializes.
		const TWeakObjectPtr<AActor> ActorPtr(Actor);
		if (!PendingActors.Contains(ActorPtr))
		{
			PendingActors.Add(ActorPtr);
			PawnExt->OnAbilitySystemInitialized.AddWeakLambda(this, [this, ActorPtr]()
			{
				if (PendingActors.Remove(ActorPtr) > 0)
				{
					TryGrantForActor(ActorPtr.Get());
				}
			});
		}
		return;
	}

	TArray<FAtlasAbilitySetHandle> Handles;
	for (const TSoftObjectPtr<UAtlasAbilitySet>& SetPtr : AbilitySets)
	{
		if (const UAtlasAbilitySet* AbilitySet = SetPtr.LoadSynchronous())
		{
			FAtlasAbilitySetHandle Handle = ASC->GrantAbilitySet(AbilitySet);
			if (Handle.IsValid())
			{
				Handles.Add(MoveTemp(Handle));
			}
		}
	}

	if (Handles.Num() > 0)
	{
		GrantedHandles.Add(Actor, MoveTemp(Handles));
	}
}

void UAtlasGameFeatureAction_AddAbilities::RevokeForActor(AActor* Actor)
{
	TArray<FAtlasAbilitySetHandle> Handles;
	if (!GrantedHandles.RemoveAndCopyValue(Actor, Handles))
	{
		return;
	}

	const UAtlasPawnExtensionComponent* PawnExt = UAtlasPawnExtensionComponent::FindPawnExtensionComponent(Cast<APawn>(Actor));
	UAtlasAbilitySystemComponent* ASC = PawnExt != nullptr ? PawnExt->GetAtlasAbilitySystemComponent() : nullptr;
	if (ASC == nullptr)
	{
		return;
	}

	for (FAtlasAbilitySetHandle& Handle : Handles)
	{
		ASC->RevokeAbilitySet(Handle);
	}
}

void UAtlasGameFeatureAction_AddAbilities::Reset()
{
	for (auto It = GrantedHandles.CreateIterator(); It; ++It)
	{
		if (AActor* Actor = It.Key().Get())
		{
			TArray<FAtlasAbilitySetHandle> Handles = MoveTemp(It.Value());
			It.RemoveCurrent();

			const UAtlasPawnExtensionComponent* PawnExt = UAtlasPawnExtensionComponent::FindPawnExtensionComponent(Cast<APawn>(Actor));
			if (UAtlasAbilitySystemComponent* ASC = PawnExt != nullptr ? PawnExt->GetAtlasAbilitySystemComponent() : nullptr)
			{
				for (FAtlasAbilitySetHandle& Handle : Handles)
				{
					ASC->RevokeAbilitySet(Handle);
				}
			}
		}
	}

	GrantedHandles.Empty();
	PendingActors.Empty();
	ExtensionRequestHandles.Empty();
}
