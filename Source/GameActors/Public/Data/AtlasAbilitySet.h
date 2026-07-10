#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "AtlasAbilitySet.generated.h"

class UAtlasBaseAttributeSet;
class UAtlasBaseGameplayAbility;
class UAttributeSet;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct FAtlasGrantedAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSoftClassPtr<UAtlasBaseGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	int32 AbilityLevel = 1;

	/*
	 * Input tag that activates this ability (see Atlas.Input.Ability.*).
	 * Added to the spec's dynamic source tags on grant.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (Categories = "Atlas.Input"))
	FGameplayTag InputTag;
};

USTRUCT(BlueprintType)
struct FAtlasGrantedGameplayEffect
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSoftClassPtr<UGameplayEffect> EffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float Level = 1.0f;
};

/*
 * Tracks everything granted by one GrantAbilitySet call so it can be revoked
 * as a unit. C++-only bookkeeping type.
 */
struct FAtlasAbilitySetHandle
{
	TArray<FGameplayAbilitySpecHandle> AbilityHandles;
	TArray<FActiveGameplayEffectHandle> EffectHandles;
	TArray<TWeakObjectPtr<UAttributeSet>> GrantedAttributeSets;

	bool IsValid() const
	{
		return AbilityHandles.Num() > 0 || EffectHandles.Num() > 0 || GrantedAttributeSets.Num() > 0;
	}
};

/*
 * Data-driven bundle of abilities, startup effects, and attribute sets to
 * grant to a pawn. Referenced by UAtlasPawnData; granted through
 * UAtlasAbilitySystemComponent::GrantAbilitySet.
 *
 * Primary asset type: AtlasAbilitySet.
 */
UCLASS(BlueprintType)
class GAMEACTORS_API UAtlasAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType PrimaryAssetType;

	//~UPrimaryDataAsset interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	//~End of UPrimaryDataAsset interface

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<FAtlasGrantedAbility> GrantedAbilities;

	// Added before effects so startup effects can initialize their values.
	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	TArray<TSoftClassPtr<UAtlasBaseAttributeSet>> GrantedAttributeSets;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TArray<FAtlasGrantedGameplayEffect> GrantedEffects;
};
