#include "Components/AtlasPawnExtensionComponent.h"

#include "AbilitySystemInterface.h"
#include "Data/AtlasPawnData.h"
#include "GAS/AtlasAbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Logging/AtlasLogMacros.h"

UAtlasPawnExtensionComponent::UAtlasPawnExtensionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UAtlasPawnExtensionComponent* UAtlasPawnExtensionComponent::FindPawnExtensionComponent(const APawn* Pawn)
{
	return Pawn != nullptr ? Pawn->FindComponentByClass<UAtlasPawnExtensionComponent>() : nullptr;
}

void UAtlasPawnExtensionComponent::SetPawnData(const UAtlasPawnData* InPawnData)
{
	if (AbilitySystemComponent != nullptr)
	{
		ATLAS_ENSURE_MSG(false, "SetPawnData called after the ability system was initialized on %s",
			*GetNameSafe(GetOwner()));
		return;
	}

	PawnData = InPawnData;
}

void UAtlasPawnExtensionComponent::HandleControllerChanged()
{
	APawn* Pawn = GetOwningPawn();
	if (Pawn == nullptr)
	{
		return;
	}

	// Player-controlled pawns use the PlayerState-owned ASC so abilities and
	// attributes survive pawn destruction and respawn.
	if (APlayerState* PlayerState = Pawn->GetPlayerState())
	{
		if (const IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(PlayerState))
		{
			if (UAtlasAbilitySystemComponent* ASC = Cast<UAtlasAbilitySystemComponent>(AbilityInterface->GetAbilitySystemComponent()))
			{
				InitializeAbilitySystem(ASC, PlayerState);
				return;
			}
		}
	}

	// Unpossessed (no player state): tear down a PlayerState-owned binding.
	// AI-owned ASCs are torn down explicitly by the AI controller.
	if (AbilitySystemComponent != nullptr && AbilitySystemComponent->GetOwner() != Pawn && Pawn->GetController() == nullptr)
	{
		UninitializeAbilitySystem();
	}
}

void UAtlasPawnExtensionComponent::InitializeAbilitySystem(UAtlasAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	if (!ATLAS_ENSURE(InASC != nullptr) || !ATLAS_ENSURE(InOwnerActor != nullptr))
	{
		return;
	}

	APawn* Pawn = GetOwningPawn();
	if (Pawn == nullptr)
	{
		return;
	}

	if (AbilitySystemComponent == InASC && InASC->GetAvatarActor() == Pawn)
	{
		return;
	}

	if (AbilitySystemComponent != nullptr && AbilitySystemComponent != InASC)
	{
		UninitializeAbilitySystem();
	}

	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

	if (Pawn->HasAuthority())
	{
		GrantPawnDataAbilitySets();
	}

	OnAbilitySystemInitialized.Broadcast();

	ATLAS_LOG_ACTORS(Log, "Ability system initialized for %s (owner %s)",
		*Pawn->GetName(), *InOwnerActor->GetName());
}

void UAtlasPawnExtensionComponent::UninitializeAbilitySystem()
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	RevokeGrantedAbilitySets();

	// Release the avatar only if this pawn still is the avatar; the ASC may
	// already belong to another pawn after a respawn.
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		AbilitySystemComponent->SetAvatarActor(nullptr);
	}

	AbilitySystemComponent = nullptr;
	OnAbilitySystemUninitialized.Broadcast();
}

void UAtlasPawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();

	Super::EndPlay(EndPlayReason);
}

APawn* UAtlasPawnExtensionComponent::GetOwningPawn() const
{
	return Cast<APawn>(GetOwner());
}

void UAtlasPawnExtensionComponent::GrantPawnDataAbilitySets()
{
	if (PawnData == nullptr || AbilitySystemComponent == nullptr)
	{
		return;
	}

	for (const TSoftObjectPtr<UAtlasAbilitySet>& SetPtr : PawnData->AbilitySets)
	{
		if (const UAtlasAbilitySet* AbilitySet = SetPtr.LoadSynchronous())
		{
			FAtlasAbilitySetHandle Handle = AbilitySystemComponent->GrantAbilitySet(AbilitySet);
			if (Handle.IsValid())
			{
				GrantedSetHandles.Add(MoveTemp(Handle));
			}
		}
	}
}

void UAtlasPawnExtensionComponent::RevokeGrantedAbilitySets()
{
	if (AbilitySystemComponent != nullptr)
	{
		for (FAtlasAbilitySetHandle& Handle : GrantedSetHandles)
		{
			AbilitySystemComponent->RevokeAbilitySet(Handle);
		}
	}

	GrantedSetHandles.Reset();
}
