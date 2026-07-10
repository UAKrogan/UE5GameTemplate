// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AtlasPlayerController.generated.h"

class UAtlasInputExtensionComponent;

/*
 * Base player controller. Owns the input extension component, which applies
 * pawn data mapping contexts and binds ability input on pawn restart.
 */
UCLASS()
class GAMEACTORS_API AAtlasPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAtlasPlayerController();

	UAtlasInputExtensionComponent* GetInputExtensionComponent() const { return InputExtComp; }

protected:
	//~AActor interface (Modular Gameplay receiver)
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	virtual void SetupInputComponent() override;

	//~AController interface
	// OnUnPossess removes the input extension's mapping contexts; ability
	// bindings die with the pawn's input component automatically.
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	//~End of AController interface

	/*
	 * Game-project hook called from BeginPlay after the framework setup.
	 */
	virtual void InitializeController();

	/*
	 * Game-project hook called from SetupInputComponent for controller-level
	 * (non-pawn) bindings, e.g. pause. Pawn input flows through InputExtComp.
	 */
	virtual void InitializeInput();

	// Applies pawn data mapping contexts and binds ability input actions on
	// pawn restart; cleaned up on unpossess.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atlas")
	TObjectPtr<UAtlasInputExtensionComponent> InputExtComp;
};
