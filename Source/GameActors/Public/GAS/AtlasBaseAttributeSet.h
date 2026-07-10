#pragma once

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "AtlasBaseAttributeSet.generated.h"

/*
 * Standard accessor bundle for Atlas attributes.
 */
#define ATLAS_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/*
 * Minimal shared attribute set base.
 *
 * Deliberately contains no game-specific attributes (health, stamina, ...) —
 * game projects subclass this and add their own. Provides the standard
 * clamp/override pattern and one framework-level example attribute (Level).
 */
UCLASS()
class GAMEACTORS_API UAtlasBaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	//~UAttributeSet interface

	/*
	 * Clamp values before they are applied. Subclasses adding attributes
	 * should call Super first, then clamp their own attributes here.
	 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/*
	 * React to instant effect execution (e.g. convert incoming Damage meta
	 * attribute into Health changes). Empty at the framework level.
	 */
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UAttributeSet interface

	ATLAS_ATTRIBUTE_ACCESSORS(UAtlasBaseAttributeSet, Level);

protected:
	/*
	 * Shared clamp helper for PreAttributeChange implementations.
	 */
	static float ClampAttributeOnChange(float NewValue, float Min, float Max);

	UFUNCTION()
	virtual void OnRep_Level(const FGameplayAttributeData& OldLevel);

	/*
	 * Example framework-level attribute demonstrating the full replicated
	 * attribute pattern (rep-notify + accessors). Not game-specific.
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Level, Category = "Attributes")
	FGameplayAttributeData Level;
};
