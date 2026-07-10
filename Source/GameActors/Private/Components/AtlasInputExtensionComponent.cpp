#include "Components/AtlasInputExtensionComponent.h"

#include "Components/AtlasPawnExtensionComponent.h"
#include "Data/AtlasInputConfigData.h"
#include "Data/AtlasPawnData.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GAS/AtlasAbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "Logging/AtlasLogMacros.h"

UAtlasInputExtensionComponent::UAtlasInputExtensionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAtlasInputExtensionComponent::InitializeForPawn(APawn* InPawn, UInputComponent* PawnInputComponent)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!ATLAS_ENSURE(PlayerController != nullptr) || !ATLAS_ENSURE(InPawn != nullptr))
	{
		return;
	}

	const UAtlasPawnExtensionComponent* PawnExt = UAtlasPawnExtensionComponent::FindPawnExtensionComponent(InPawn);
	const UAtlasPawnData* PawnData = PawnExt != nullptr ? PawnExt->GetPawnData() : nullptr;
	if (PawnData == nullptr)
	{
		return;
	}

	Cleanup();
	BoundPawn = InPawn;

	for (const TSoftObjectPtr<UAtlasInputConfigData>& ConfigPtr : PawnData->InputConfigs)
	{
		ApplyConfig(ConfigPtr.LoadSynchronous());
	}

	ATLAS_LOG_ACTORS(Log, "Input initialized for %s (%d configs)", *InPawn->GetName(), AppliedConfigs.Num());
}

void UAtlasInputExtensionComponent::ApplyConfig(const UAtlasInputConfigData* Config)
{
	if (Config == nullptr || AppliedConfigs.Contains(Config))
	{
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (PlayerController == nullptr)
	{
		return;
	}

	APawn* Pawn = BoundPawn.Get();
	if (Pawn == nullptr)
	{
		Pawn = PlayerController->GetPawn();
		BoundPawn = Pawn;
	}

	AppliedConfigs.Add(Config);

	if (const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (UInputMappingContext* MappingContext = Config->MappingContext.LoadSynchronous())
			{
				InputSubsystem->AddMappingContext(MappingContext, Config->Priority);
			}
		}
	}

	// Bindings attach to the pawn's input component, so they are torn down
	// automatically when the pawn is unpossessed or destroyed.
	UEnhancedInputComponent* EnhancedInput = Pawn != nullptr ? Cast<UEnhancedInputComponent>(Pawn->InputComponent) : nullptr;
	if (EnhancedInput == nullptr)
	{
		return;
	}

	for (const FAtlasAbilityInputBinding& Binding : Config->AbilityBindings)
	{
		const UInputAction* InputAction = Binding.InputAction.LoadSynchronous();
		if (InputAction == nullptr || !Binding.InputTag.IsValid())
		{
			continue;
		}

		EnhancedInput->BindAction(InputAction, ETriggerEvent::Started, this,
			&UAtlasInputExtensionComponent::HandleAbilityInputPressed, Binding.InputTag);
		EnhancedInput->BindAction(InputAction, ETriggerEvent::Completed, this,
			&UAtlasInputExtensionComponent::HandleAbilityInputReleased, Binding.InputTag);
	}
}

void UAtlasInputExtensionComponent::RemoveConfig(const UAtlasInputConfigData* Config)
{
	if (Config == nullptr || AppliedConfigs.Remove(const_cast<UAtlasInputConfigData*>(Config)) == 0)
	{
		return;
	}

	if (const APlayerController* PlayerController = Cast<APlayerController>(GetOwner()))
	{
		if (const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (UInputMappingContext* MappingContext = Config->MappingContext.Get())
				{
					InputSubsystem->RemoveMappingContext(MappingContext);
				}
			}
		}
	}
}

void UAtlasInputExtensionComponent::Cleanup()
{
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetOwner()))
	{
		if (const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				for (const UAtlasInputConfigData* Config : AppliedConfigs)
				{
					if (UInputMappingContext* MappingContext = Config->MappingContext.Get())
					{
						InputSubsystem->RemoveMappingContext(MappingContext);
					}
				}
			}
		}
	}

	AppliedConfigs.Reset();
	BoundPawn.Reset();
}

void UAtlasInputExtensionComponent::HandleAbilityInputPressed(FGameplayTag InputTag)
{
	if (const UAtlasPawnExtensionComponent* PawnExt = UAtlasPawnExtensionComponent::FindPawnExtensionComponent(BoundPawn.Get()))
	{
		if (UAtlasAbilitySystemComponent* ASC = PawnExt->GetAtlasAbilitySystemComponent())
		{
			ASC->AbilityInputTagPressed(InputTag);
		}
	}
}

void UAtlasInputExtensionComponent::HandleAbilityInputReleased(FGameplayTag InputTag)
{
	if (const UAtlasPawnExtensionComponent* PawnExt = UAtlasPawnExtensionComponent::FindPawnExtensionComponent(BoundPawn.Get()))
	{
		if (UAtlasAbilitySystemComponent* ASC = PawnExt->GetAtlasAbilitySystemComponent())
		{
			ASC->AbilityInputTagReleased(InputTag);
		}
	}
}
