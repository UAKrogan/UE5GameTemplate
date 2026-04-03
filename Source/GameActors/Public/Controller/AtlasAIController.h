// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AtlasAIController.generated.h"

/**
 * 
 */
UCLASS()
class GAMEACTORS_API AAtlasAIController : public AAIController
{
	GENERATED_BODY()

public:
	AAtlasAIController();

protected:
	virtual void BeginPlay() override;

	virtual void InitializeAI();
};
