#include "GAS/AtlasBaseAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

void UAtlasBaseAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetLevelAttribute())
	{
		NewValue = ClampAttributeOnChange(NewValue, 1.0f, TNumericLimits<float>::Max());
	}
}

void UAtlasBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UAtlasBaseAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAtlasBaseAttributeSet, Level, COND_None, REPNOTIFY_Always);
}

float UAtlasBaseAttributeSet::ClampAttributeOnChange(float NewValue, float Min, float Max)
{
	return FMath::Clamp(NewValue, Min, Max);
}

void UAtlasBaseAttributeSet::OnRep_Level(const FGameplayAttributeData& OldLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtlasBaseAttributeSet, Level, OldLevel);
}
