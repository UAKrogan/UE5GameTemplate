// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AtlasGameState.generated.h"

/**
 * 
 */
UCLASS()
class GAMEACTORS_API AAtlasGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AAtlasGameState();

protected:
	virtual void BeginPlay() override;

	virtual void InitializeGameState();
};
