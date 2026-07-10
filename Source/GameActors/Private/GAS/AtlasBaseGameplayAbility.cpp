#include "GAS/AtlasBaseGameplayAbility.h"

#include "Controller/AtlasPlayerController.h"
#include "GAS/AtlasAbilitySystemComponent.h"
#include "GameFramework/Pawn.h"

UAtlasBaseGameplayAbility::UAtlasBaseGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

APawn* UAtlasBaseGameplayAbility::GetAvatarPawn() const
{
	return Cast<APawn>(GetAvatarActorFromActorInfo());
}

AAtlasPlayerController* UAtlasBaseGameplayAbility::GetAtlasPlayerController() const
{
	if (const APawn* Pawn = GetAvatarPawn())
	{
		return Cast<AAtlasPlayerController>(Pawn->GetController());
	}

	return nullptr;
}

UAtlasAbilitySystemComponent* UAtlasBaseGameplayAbility::GetAtlasAbilitySystemComponent() const
{
	return Cast<UAtlasAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
}
