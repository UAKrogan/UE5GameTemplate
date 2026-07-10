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
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	//~AController interface
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	//~End of AController interface

	virtual void InitializeController();

	virtual void InitializeInput();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atlas")
	TObjectPtr<UAtlasInputExtensionComponent> InputExtComp;
};
