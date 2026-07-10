#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Systems/Savable/AtlasSavable.h"
#include "AtlasGASSaveAdapter.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;

/*
 * One captured attribute value: identified by attribute set class path +
 * attribute name so restore works across sessions without object pointers.
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FAtlasGASAttributeSaveData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	FString AttributeSetClassPath;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	FString AttributeName;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	float BaseValue = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	float CurrentValue = 0.0f;
};

/*
 * One captured active gameplay effect, including enough timing/stack data to
 * re-apply it with the remaining duration after a load.
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FAtlasGASEffectSaveData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	FString EffectClassPath;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	float Level = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	int32 StackCount = 1;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	float Duration = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	float TimeRemaining = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	FGameplayTagContainer AssetTags;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	FGameplayTagContainer GrantedTags;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	bool bLikelyCooldown = false;
};

/*
 * Gameplay Ability System save adapter.
 *
 * This component discovers an AbilitySystemComponent on its owning actor,
 * captures attributes and active gameplay effects, and restores them back to
 * that same actor. It has no dependency on FAtlasSaveSystem.
 */
UCLASS(ClassGroup = (Atlas), meta = (BlueprintSpawnableComponent))
class GAMECORE_API UAtlasGASSaveAdapter : public UActorComponent, public IAtlasSavable
{
	GENERATED_BODY()

public:
	UAtlasGASSaveAdapter();

	virtual void CaptureState(FAtlasSaveContext& Context) override;
	virtual void RestoreState(const FAtlasLoadContext& Context) override;

private:
	UAbilitySystemComponent* ResolveAbilitySystemComponent() const;

	void CaptureAttributes(UAbilitySystemComponent* AbilitySystemComponent, TArray<FAtlasGASAttributeSaveData>& OutAttributes) const;
	void CaptureEffects(UAbilitySystemComponent* AbilitySystemComponent, TArray<FAtlasGASEffectSaveData>& OutEffects) const;
	void RestoreAttributes(UAbilitySystemComponent* AbilitySystemComponent, const TArray<FAtlasGASAttributeSaveData>& Attributes) const;
	void RestoreEffects(UAbilitySystemComponent* AbilitySystemComponent, const TArray<FAtlasGASEffectSaveData>& Effects) const;

	void SerializeAttribute(FArchive& Archive, FAtlasGASAttributeSaveData& Attribute) const;
	void SerializeEffect(FArchive& Archive, FAtlasGASEffectSaveData& Effect) const;
	void SerializeTagContainer(FArchive& Archive, FGameplayTagContainer& TagContainer) const;

	bool IsLikelyCooldownEffect(const FGameplayTagContainer& AssetTags, const FGameplayTagContainer& GrantedTags) const;
};
