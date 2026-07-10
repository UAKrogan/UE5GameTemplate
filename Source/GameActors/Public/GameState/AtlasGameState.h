// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "AtlasGameState.generated.h"

/*
 * Base game state. Extends AGameState (not AGameStateBase) to match
 * AAtlasGameMode's AGameMode base, which requires match state replication.
 */
UCLASS()
class GAMEACTORS_API AAtlasGameState : public AGameState
{
	GENERATED_BODY()

public:
	AAtlasGameState();

protected:
	//~AActor interface (Modular Gameplay receiver)
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	virtual void InitializeGameState();
};
