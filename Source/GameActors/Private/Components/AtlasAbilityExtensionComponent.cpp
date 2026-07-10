#include "Components/AtlasAbilityExtensionComponent.h"

#include "GAS/AtlasAbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "Logging/AtlasLogMacros.h"

UAtlasAbilityExtensionComponent::UAtlasAbilityExtensionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UAtlasAbilityExtensionComponent* UAtlasAbilityExtensionComponent::FindAbilityExtensionComponent(const APawn* Pawn)
{
	return Pawn != nullptr ? Pawn->FindComponentByClass<UAtlasAbilityExtensionComponent>() : nullptr;
}

UAtlasAbilitySystemComponent* UAtlasAbilityExtensionComponent::GetOrCreateAbilitySystemComponent()
{
	if (AbilitySystemComponent != nullptr)
	{
		return AbilitySystemComponent;
	}

	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return nullptr;
	}

	AbilitySystemComponent = NewObject<UAtlasAbilitySystemComponent>(Owner, TEXT("AtlasAbilitySystemComponent"));
	AbilitySystemComponent->RegisterComponent();

	ATLAS_LOG_ACTORS(Log, "Pawn-owned ASC created for %s", *Owner->GetName());

	return AbilitySystemComponent;
}

FAtlasAbilitySetHandle UAtlasAbilityExtensionComponent::GrantAbilitySet(const UAtlasAbilitySet* AbilitySet, UAtlasAbilitySystemComponent* ASC)
{
	if (!ATLAS_ENSURE(ASC != nullptr))
	{
		return FAtlasAbilitySetHandle();
	}

	return ASC->GrantAbilitySet(AbilitySet);
}

void UAtlasAbilityExtensionComponent::RevokeAbilitySet(FAtlasAbilitySetHandle& Handle, UAtlasAbilitySystemComponent* ASC)
{
	if (ASC != nullptr)
	{
		ASC->RevokeAbilitySet(Handle);
	}
}
