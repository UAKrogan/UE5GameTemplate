// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AtlasPlayerController.generated.h"

/*
 * Base player controller. Possession hooks are overridable for game
 * projects; Enhanced Input wiring is added via the input extension
 * component (Phase 6).
 */
UCLASS()
class GAMEACTORS_API AAtlasPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAtlasPlayerController();

protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	//~AController interface
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	//~End of AController interface

	virtual void InitializeController();

	virtual void InitializeInput();
};
