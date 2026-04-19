#include "Systems/Adapters/AtlasGASSaveAdapter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AttributeSet.h"
#include "GameplayEffect.h"
#include "Logging/AtlasLogMacros.h"

namespace AtlasGASSaveAdapter
{
	constexpr int32 DataVersion = 1;
}

UAtlasGASSaveAdapter::UAtlasGASSaveAdapter()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
}

void UAtlasGASSaveAdapter::CaptureState(FAtlasSaveContext& Context)
{
	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		ATLAS_LOG_CORE(Warning, "GAS save adapter capture skipped: owner has no AbilitySystemComponent owner=%s", *GetNameSafe(GetOwner()));
		return;
	}

	TArray<FAtlasGASAttributeSaveData> Attributes;
	TArray<FAtlasGASEffectSaveData> Effects;

	CaptureAttributes(AbilitySystemComponent, Attributes);
	CaptureEffects(AbilitySystemComponent, Effects);

	FArchive& Archive = Context.GetArchive();
	int32 Version = AtlasGASSaveAdapter::DataVersion;
	int32 AttributeCount = Attributes.Num();
	int32 EffectCount = Effects.Num();

	Archive << Version;
	Archive << AttributeCount;
	for (FAtlasGASAttributeSaveData& Attribute : Attributes)
	{
		SerializeAttribute(Archive, Attribute);
	}

	Archive << EffectCount;
	for (FAtlasGASEffectSaveData& Effect : Effects)
	{
		SerializeEffect(Archive, Effect);
	}

	ATLAS_LOG_CORE(Verbose, "Captured GAS adapter state: owner=%s attributes=%d effects=%d",
		*GetNameSafe(GetOwner()),
		Attributes.Num(),
		Effects.Num());
}

void UAtlasGASSaveAdapter::RestoreState(const FAtlasLoadContext& Context)
{
	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		ATLAS_LOG_CORE(Warning, "GAS save adapter restore skipped: owner has no AbilitySystemComponent owner=%s", *GetNameSafe(GetOwner()));
		return;
	}

	FArchive& Archive = const_cast<FAtlasLoadContext&>(Context).GetArchive();
	int32 Version = 0;
	int32 AttributeCount = 0;
	int32 EffectCount = 0;

	Archive << Version;
	Archive << AttributeCount;

	if (Archive.IsError() || Version <= 0 || AttributeCount < 0)
	{
		ATLAS_LOG_CORE(Error, "Failed to restore GAS adapter attributes: owner=%s", *GetNameSafe(GetOwner()));
		return;
	}

	TArray<FAtlasGASAttributeSaveData> Attributes;
	Attributes.SetNum(AttributeCount);
	for (FAtlasGASAttributeSaveData& Attribute : Attributes)
	{
		SerializeAttribute(Archive, Attribute);
	}

	Archive << EffectCount;
	if (Archive.IsError() || EffectCount < 0)
	{
		ATLAS_LOG_CORE(Error, "Failed to restore GAS adapter effects: owner=%s", *GetNameSafe(GetOwner()));
		return;
	}

	TArray<FAtlasGASEffectSaveData> Effects;
	Effects.SetNum(EffectCount);
	for (FAtlasGASEffectSaveData& Effect : Effects)
	{
		SerializeEffect(Archive, Effect);
	}

	RestoreAttributes(AbilitySystemComponent, Attributes);
	RestoreEffects(AbilitySystemComponent, Effects);

	ATLAS_LOG_CORE(Verbose, "Restored GAS adapter state: owner=%s attributes=%d effects=%d",
		*GetNameSafe(GetOwner()),
		Attributes.Num(),
		Effects.Num());
}

UAbilitySystemComponent* UAtlasGASSaveAdapter::ResolveAbilitySystemComponent() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	if (IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(Owner))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface->GetAbilitySystemComponent())
		{
			return AbilitySystemComponent;
		}
	}

	return Owner->FindComponentByClass<UAbilitySystemComponent>();
}

void UAtlasGASSaveAdapter::CaptureAttributes(UAbilitySystemComponent* AbilitySystemComponent, TArray<FAtlasGASAttributeSaveData>& OutAttributes) const
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	const TArray<UAttributeSet*>& AttributeSets = AbilitySystemComponent->GetSpawnedAttributes();
	for (const UAttributeSet* AttributeSet : AttributeSets)
	{
		if (!AttributeSet)
		{
			continue;
		}

		TArray<FGameplayAttribute> GameplayAttributes;
		UAttributeSet::GetAttributesFromSetClass(AttributeSet->GetClass(), GameplayAttributes);

		for (const FGameplayAttribute& GameplayAttribute : GameplayAttributes)
		{
			if (!GameplayAttribute.IsValid())
			{
				continue;
			}

			FAtlasGASAttributeSaveData& AttributeData = OutAttributes.Emplace_GetRef();
			AttributeData.AttributeSetClassPath = AttributeSet->GetClass()->GetPathName();
			AttributeData.AttributeName = GameplayAttribute.GetName();
			AttributeData.CurrentValue = GameplayAttribute.GetNumericValue(AttributeSet);

			if (const FGameplayAttributeData* GameplayAttributeData = GameplayAttribute.GetGameplayAttributeData(AttributeSet))
			{
				AttributeData.BaseValue = GameplayAttributeData->GetBaseValue();
			}
			else
			{
				AttributeData.BaseValue = AttributeData.CurrentValue;
			}
		}
	}
}

void UAtlasGASSaveAdapter::CaptureEffects(UAbilitySystemComponent* AbilitySystemComponent, TArray<FAtlasGASEffectSaveData>& OutEffects) const
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	const float WorldTimeSeconds = AbilitySystemComponent->GetWorld() ? AbilitySystemComponent->GetWorld()->GetTimeSeconds() : 0.0f;
	const TArray<FActiveGameplayEffectHandle> ActiveEffectHandles = AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery());

	for (const FActiveGameplayEffectHandle& ActiveEffectHandle : ActiveEffectHandles)
	{
		const FActiveGameplayEffect* ActiveGameplayEffect = AbilitySystemComponent->GetActiveGameplayEffect(ActiveEffectHandle);
		if (!ActiveGameplayEffect || !ActiveGameplayEffect->Spec.Def)
		{
			continue;
		}

		FAtlasGASEffectSaveData& EffectData = OutEffects.Emplace_GetRef();
		EffectData.EffectClassPath = ActiveGameplayEffect->Spec.Def->GetClass()->GetPathName();
		EffectData.Level = ActiveGameplayEffect->Spec.GetLevel();
		EffectData.StackCount = ActiveGameplayEffect->Spec.GetStackCount();
		EffectData.Duration = ActiveGameplayEffect->GetDuration();
		EffectData.TimeRemaining = ActiveGameplayEffect->GetTimeRemaining(WorldTimeSeconds);
		ActiveGameplayEffect->Spec.GetAllAssetTags(EffectData.AssetTags);
		ActiveGameplayEffect->Spec.GetAllGrantedTags(EffectData.GrantedTags);
		EffectData.bLikelyCooldown = IsLikelyCooldownEffect(EffectData.AssetTags, EffectData.GrantedTags);
	}
}

void UAtlasGASSaveAdapter::RestoreAttributes(UAbilitySystemComponent* AbilitySystemComponent, const TArray<FAtlasGASAttributeSaveData>& Attributes) const
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	const TArray<UAttributeSet*>& AttributeSets = AbilitySystemComponent->GetSpawnedAttributes();
	for (const FAtlasGASAttributeSaveData& SavedAttribute : Attributes)
	{
		UAttributeSet* MatchingAttributeSet = nullptr;
		for (UAttributeSet* AttributeSet : AttributeSets)
		{
			if (AttributeSet && AttributeSet->GetClass()->GetPathName() == SavedAttribute.AttributeSetClassPath)
			{
				MatchingAttributeSet = AttributeSet;
				break;
			}
		}

		if (!MatchingAttributeSet)
		{
			ATLAS_LOG_CORE(Warning, "Could not restore GAS attribute: missing set owner=%s set=%s attribute=%s",
				*GetNameSafe(GetOwner()),
				*SavedAttribute.AttributeSetClassPath,
				*SavedAttribute.AttributeName);
			continue;
		}

		TArray<FGameplayAttribute> GameplayAttributes;
		UAttributeSet::GetAttributesFromSetClass(MatchingAttributeSet->GetClass(), GameplayAttributes);

		for (const FGameplayAttribute& GameplayAttribute : GameplayAttributes)
		{
			if (GameplayAttribute.GetName() != SavedAttribute.AttributeName)
			{
				continue;
			}

			AbilitySystemComponent->SetNumericAttributeBase(GameplayAttribute, SavedAttribute.BaseValue);
			float CurrentValue = SavedAttribute.CurrentValue;
			GameplayAttribute.SetNumericValueChecked(CurrentValue, MatchingAttributeSet);
			break;
		}
	}
}

void UAtlasGASSaveAdapter::RestoreEffects(UAbilitySystemComponent* AbilitySystemComponent, const TArray<FAtlasGASEffectSaveData>& Effects) const
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	for (const FAtlasGASEffectSaveData& SavedEffect : Effects)
	{
		if (SavedEffect.EffectClassPath.IsEmpty())
		{
			continue;
		}

		UClass* EffectClassObject = LoadObject<UClass>(nullptr, *SavedEffect.EffectClassPath);
		if (!EffectClassObject || !EffectClassObject->IsChildOf(UGameplayEffect::StaticClass()))
		{
			ATLAS_LOG_CORE(Warning, "Could not restore GAS effect: missing class owner=%s class=%s",
				*GetNameSafe(GetOwner()),
				*SavedEffect.EffectClassPath);
			continue;
		}

		const TSubclassOf<UGameplayEffect> EffectClass(EffectClassObject);
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(GetOwner());

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, SavedEffect.Level, EffectContext);
		if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
		{
			continue;
		}

		SpecHandle.Data->SetStackCount(FMath::Max(1, SavedEffect.StackCount));
		SpecHandle.Data->AppendDynamicAssetTags(SavedEffect.AssetTags);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		if (SavedEffect.Duration > 0.0f && SavedEffect.TimeRemaining >= 0.0f)
		{
			ATLAS_LOG_CORE(Verbose, "Restored GAS effect with fresh duration: owner=%s class=%s savedRemaining=%.2f",
				*GetNameSafe(GetOwner()),
				*SavedEffect.EffectClassPath,
				SavedEffect.TimeRemaining);
		}
	}
}

void UAtlasGASSaveAdapter::SerializeAttribute(FArchive& Archive, FAtlasGASAttributeSaveData& Attribute) const
{
	Archive << Attribute.AttributeSetClassPath;
	Archive << Attribute.AttributeName;
	Archive << Attribute.BaseValue;
	Archive << Attribute.CurrentValue;
}

void UAtlasGASSaveAdapter::SerializeEffect(FArchive& Archive, FAtlasGASEffectSaveData& Effect) const
{
	Archive << Effect.EffectClassPath;
	Archive << Effect.Level;
	Archive << Effect.StackCount;
	Archive << Effect.Duration;
	Archive << Effect.TimeRemaining;
	SerializeTagContainer(Archive, Effect.AssetTags);
	SerializeTagContainer(Archive, Effect.GrantedTags);
	Archive << Effect.bLikelyCooldown;
}

void UAtlasGASSaveAdapter::SerializeTagContainer(FArchive& Archive, FGameplayTagContainer& TagContainer) const
{
	if (Archive.IsSaving())
	{
		TArray<FGameplayTag> Tags;
		TagContainer.GetGameplayTagArray(Tags);

		int32 TagCount = Tags.Num();
		Archive << TagCount;

		for (const FGameplayTag& Tag : Tags)
		{
			FString TagName = Tag.ToString();
			Archive << TagName;
		}
	}
	else
	{
		int32 TagCount = 0;
		Archive << TagCount;

		TagContainer.Reset();
		for (int32 TagIndex = 0; TagIndex < TagCount; ++TagIndex)
		{
			FString TagName;
			Archive << TagName;

			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*TagName), false);
			if (Tag.IsValid())
			{
				TagContainer.AddTag(Tag);
			}
		}
	}
}

bool UAtlasGASSaveAdapter::IsLikelyCooldownEffect(const FGameplayTagContainer& AssetTags, const FGameplayTagContainer& GrantedTags) const
{
	TArray<FGameplayTag> Tags;
	AssetTags.GetGameplayTagArray(Tags);
	GrantedTags.GetGameplayTagArray(Tags);

	for (const FGameplayTag& Tag : Tags)
	{
		if (Tag.ToString().Contains(TEXT("Cooldown"), ESearchCase::IgnoreCase))
		{
			return true;
		}
	}

	return false;
}
