#include "GAS/AtlasAbilitySystemComponent.h"

#include "GAS/AtlasBaseAttributeSet.h"
#include "GAS/AtlasBaseGameplayAbility.h"
#include "GameplayEffect.h"
#include "Logging/AtlasLogMacros.h"

UAtlasAbilitySystemComponent::UAtlasAbilitySystemComponent()
{
	SetIsReplicatedByDefault(true);
	ReplicationMode = EGameplayEffectReplicationMode::Mixed;
}

void UAtlasAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability == nullptr || !Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		AbilitySpecInputPressed(Spec);

		if (!Spec.IsActive())
		{
			TryActivateAbility(Spec.Handle);
		}
	}
}

void UAtlasAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability == nullptr || !Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		AbilitySpecInputReleased(Spec);
	}
}

FAtlasAbilitySetHandle UAtlasAbilitySystemComponent::GrantAbilitySet(const UAtlasAbilitySet* AbilitySet)
{
	FAtlasAbilitySetHandle Handle;

	if (!ATLAS_ENSURE(AbilitySet != nullptr))
	{
		return Handle;
	}

	if (!IsOwnerActorAuthoritative())
	{
		ATLAS_LOG_ACTORS(Warning, "GrantAbilitySet(%s) called without authority", *AbilitySet->GetName());
		return Handle;
	}

	// Attribute sets first so startup effects can initialize their values.
	for (const TSoftClassPtr<UAtlasBaseAttributeSet>& SetClassPtr : AbilitySet->GrantedAttributeSets)
	{
		UClass* SetClass = SetClassPtr.LoadSynchronous();
		if (SetClass == nullptr)
		{
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(GetOwner(), SetClass);
		AddSpawnedAttribute(NewSet);
		Handle.GrantedAttributeSets.Add(NewSet);
	}

	for (const FAtlasGrantedGameplayEffect& Effect : AbilitySet->GrantedEffects)
	{
		UClass* EffectClass = Effect.EffectClass.LoadSynchronous();
		if (EffectClass == nullptr)
		{
			continue;
		}

		const UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle EffectHandle = ApplyGameplayEffectToSelf(EffectCDO, Effect.Level, MakeEffectContext());
		if (EffectHandle.IsValid())
		{
			Handle.EffectHandles.Add(EffectHandle);
		}
	}

	for (const FAtlasGrantedAbility& Ability : AbilitySet->GrantedAbilities)
	{
		UClass* AbilityClass = Ability.AbilityClass.LoadSynchronous();
		if (AbilityClass == nullptr)
		{
			continue;
		}

		FGameplayAbilitySpec Spec(AbilityClass, Ability.AbilityLevel);
		if (Ability.InputTag.IsValid())
		{
			Spec.GetDynamicSpecSourceTags().AddTag(Ability.InputTag);
		}

		Handle.AbilityHandles.Add(GiveAbility(Spec));
	}

	ATLAS_LOG_ACTORS(Log, "Ability set granted: %s (%d abilities, %d effects, %d attribute sets)",
		*AbilitySet->GetName(), Handle.AbilityHandles.Num(), Handle.EffectHandles.Num(), Handle.GrantedAttributeSets.Num());

	OnAbilitiesGranted.Broadcast();

	return Handle;
}

void UAtlasAbilitySystemComponent::RevokeAbilitySet(FAtlasAbilitySetHandle& Handle)
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& AbilityHandle : Handle.AbilityHandles)
	{
		if (AbilityHandle.IsValid())
		{
			ClearAbility(AbilityHandle);
		}
	}

	for (const FActiveGameplayEffectHandle& EffectHandle : Handle.EffectHandles)
	{
		if (EffectHandle.IsValid())
		{
			RemoveActiveGameplayEffect(EffectHandle);
		}
	}

	for (const TWeakObjectPtr<UAttributeSet>& AttributeSet : Handle.GrantedAttributeSets)
	{
		if (AttributeSet.IsValid())
		{
			RemoveSpawnedAttribute(AttributeSet.Get());
		}
	}

	Handle = FAtlasAbilitySetHandle();
}
