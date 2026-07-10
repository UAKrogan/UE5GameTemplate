#pragma once

#include "Abilities/GameplayAbility.h"
#include "AtlasBaseGameplayAbility.generated.h"

class AAtlasPlayerController;
class UAtlasAbilitySystemComponent;

/*
 * Base class for all Atlas abilities.
 *
 * Instanced per actor by default. The input tag convention lives on the
 * ability set entry (FAtlasGrantedAbility::InputTag); an ability may also
 * declare its default input tag here for self-documenting assets.
 */
UCLASS(Abstract)
class GAMEACTORS_API UAtlasBaseGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAtlasBaseGameplayAbility();

	UFUNCTION(BlueprintCallable, Category = "Atlas|Ability")
	APawn* GetAvatarPawn() const;

	UFUNCTION(BlueprintCallable, Category = "Atlas|Ability")
	AAtlasPlayerController* GetAtlasPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "Atlas|Ability")
	UAtlasAbilitySystemComponent* GetAtlasAbilitySystemComponent() const;

	/*
	 * Default input tag activating this ability (see Atlas.Input.Ability.*).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atlas|Input", meta = (Categories = "Atlas.Input"))
	FGameplayTag InputTag;
};
