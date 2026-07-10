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
		const UAtlasInputConfigData* Config = ConfigPtr.LoadSynchronous();
		ApplyConfig(Config);

		// Remember which configs are pawn defaults: those are the ones
		// suppressed while the pawn is in a vehicle or mounted.
		if (Config != nullptr)
		{
			DefaultConfigs.Add(Config);
		}
	}

	// Follow the pawn's movement mode so drive/ride transitions can swap
	// input contexts without the vehicle/mount code touching pawn defaults.
	if (UAtlasMovementExtensionComponent* MovementExt = UAtlasMovementExtensionComponent::FindMovementExtensionComponent(InPawn))
	{
		MovementExt->OnMovementModeChanged.AddDynamic(this, &UAtlasInputExtensionComponent::HandleMovementModeChanged);
		BoundMovementExt = MovementExt;
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

	if (UAtlasMovementExtensionComponent* MovementExt = BoundMovementExt.Get())
	{
		MovementExt->OnMovementModeChanged.RemoveDynamic(this, &UAtlasInputExtensionComponent::HandleMovementModeChanged);
	}

	AppliedConfigs.Reset();
	DefaultConfigs.Reset();
	BoundPawn.Reset();
	BoundMovementExt.Reset();
	bDefaultContextsSuppressed = false;
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

void UAtlasInputExtensionComponent::HandleMovementModeChanged(EAtlasMovementMode OldMode, EAtlasMovementMode NewMode)
{
	const bool bAttached =
		NewMode == EAtlasMovementMode::InVehicle || NewMode == EAtlasMovementMode::Mounted;

	SetDefaultContextsSuppressed(bAttached);
}

void UAtlasInputExtensionComponent::SetDefaultContextsSuppressed(bool bSuppressed)
{
	if (bSuppressed == bDefaultContextsSuppressed)
	{
		return;
	}
	bDefaultContextsSuppressed = bSuppressed;

	const APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	const ULocalPlayer* LocalPlayer = PlayerController != nullptr ? PlayerController->GetLocalPlayer() : nullptr;
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		LocalPlayer != nullptr ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	if (InputSubsystem == nullptr)
	{
		return;
	}

	// Only the mapping contexts toggle: ability bindings stay bound (their
	// actions simply never trigger without a context) so resuming does not
	// double-bind on the pawn's input component.
	for (const UAtlasInputConfigData* Config : DefaultConfigs)
	{
		UInputMappingContext* MappingContext = Config != nullptr ? Config->MappingContext.Get() : nullptr;
		if (MappingContext == nullptr)
		{
			continue;
		}

		if (bSuppressed)
		{
			InputSubsystem->RemoveMappingContext(MappingContext);
		}
		else
		{
			InputSubsystem->AddMappingContext(MappingContext, Config->Priority);
		}
	}

	ATLAS_LOG_ACTORS(Log, "Default input contexts %s for %s",
		bSuppressed ? TEXT("suppressed") : TEXT("resumed"), *GetNameSafe(BoundPawn.Get()));
}
