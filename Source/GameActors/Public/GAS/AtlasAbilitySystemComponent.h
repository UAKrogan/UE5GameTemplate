#pragma once

#include "AbilitySystemComponent.h"
#include "Data/AtlasAbilitySet.h"
#include "AtlasAbilitySystemComponent.generated.h"

/*
 * Atlas ability system component.
 *
 * Adds tag-based input activation and ability-set granting on top of the
 * stock ASC without changing core GAS behavior. Replication mode defaults
 * to Mixed (full to owning client, minimal to simulated proxies).
 */
UCLASS()
class GAMEACTORS_API UAtlasAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UAtlasAbilitySystemComponent();

	/*
	 * Tag-based input activation, called by the input extension component.
	 * Activates/notifies every granted ability whose dynamic source tags
	 * contain the input tag.
	 */
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	/*
	 * Grants an entire ability set (attribute sets, then effects, then
	 * abilities). Authority only. Returns a handle for RevokeAbilitySet.
	 */
	FAtlasAbilitySetHandle GrantAbilitySet(const UAtlasAbilitySet* AbilitySet);
	void RevokeAbilitySet(FAtlasAbilitySetHandle& Handle);

	// Fired after each successful GrantAbilitySet call.
	FSimpleMulticastDelegate OnAbilitiesGranted;
};
