// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "AtlasGameMode.generated.h"

/*
 * Base game mode for Atlas projects.
 *
 * Extends AGameMode (rather than AGameModeBase) to provide match state
 * support (WaitingToStart, InProgress, WaitingPostMatch) required by any
 * non-trivial game loop.
 */
UCLASS()
class GAMEACTORS_API AAtlasGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AAtlasGameMode();

	virtual void StartPlay() override;

protected:
	virtual void InitializeGameMode();
};
