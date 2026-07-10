// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "CoreMinimal.h"
#include "AtlasAIController.generated.h"

/*
 * Base AI controller. On possession, initializes a pawn-owned ability
 * system (AI pawns do not use a PlayerState-owned ASC).
 */
UCLASS()
class GAMEACTORS_API AAtlasAIController : public AAIController
{
	GENERATED_BODY()

public:
	AAtlasAIController();

protected:
	virtual void BeginPlay() override;

	//~AController interface
	// OnPossess creates/initializes the pawn-owned ASC via the pawn's
	// extension components; OnUnPossess tears it down and revokes grants.
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	//~End of AController interface

	/*
	 * Game-project hook called from BeginPlay after the framework setup
	 * (e.g. run a behavior tree).
	 */
	virtual void InitializeAI();
};
