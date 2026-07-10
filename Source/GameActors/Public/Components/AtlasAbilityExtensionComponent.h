#pragma once

#include "Components/ActorComponent.h"
#include "Data/AtlasAbilitySet.h"
#include "AtlasAbilityExtensionComponent.generated.h"

class UAtlasAbilitySystemComponent;

/*
 * Provides a pawn-owned ability system for pawns that do not use a
 * PlayerState-owned ASC (AI pawns, standalone pawns), and convenience
 * wrappers for granting/revoking ability sets.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class GAMEACTORS_API UAtlasAbilityExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasAbilityExtensionComponent();

	static UAtlasAbilityExtensionComponent* FindAbilityExtensionComponent(const APawn* Pawn);

	/*
	 * Returns the pawn-owned ASC, creating and registering it on first use.
	 * Only for AI/standalone pawns — player pawns use the PlayerState ASC.
	 */
	UAtlasAbilitySystemComponent* GetOrCreateAbilitySystemComponent();

	UFUNCTION(BlueprintCallable, Category = "Atlas|Abilities")
	UAtlasAbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystemComponent; }

	FAtlasAbilitySetHandle GrantAbilitySet(const UAtlasAbilitySet* AbilitySet, UAtlasAbilitySystemComponent* ASC);
	void RevokeAbilitySet(FAtlasAbilitySetHandle& Handle, UAtlasAbilitySystemComponent* ASC);

private:
	UPROPERTY(Transient)
	TObjectPtr<UAtlasAbilitySystemComponent> AbilitySystemComponent;
};
