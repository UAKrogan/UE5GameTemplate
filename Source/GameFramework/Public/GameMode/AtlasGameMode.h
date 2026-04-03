// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AtlasGameMode.generated.h"

/**
 * 
 */
UCLASS()
class GAMEFRAMEWORK_API AAtlasGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAtlasGameMode();

	virtual void StartPlay() override;

protected:
	virtual void InitializeGameMode();
};
