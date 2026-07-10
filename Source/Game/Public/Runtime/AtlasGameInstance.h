// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AtlasGameInstance.generated.h"

/*
 * UAtlasGameInstance
 *
 * Entry point for the Atlas runtime.
 *
 * Responsibilities:
 * - Bootstrap runtime
 * - Ensure the core subsystem exists
 * - Kick off the game flow once the engine has started the game
 *
 * Game projects may subclass and override Init/Shutdown for extra setup;
 * always call Super.
 */
UCLASS()
class GAME_API UAtlasGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	/*
	 * Verifies the core orchestrator subsystem was created. Subsystems
	 * initialize themselves before this runs.
	 */
	virtual void Init() override;

	/*
	 * Called once the engine has started the game (after Init, when a world
	 * exists). Starts the Atlas game flow: Startup -> MainMenu travel.
	 */
	virtual void OnStart() override;

	virtual void Shutdown() override;
};
