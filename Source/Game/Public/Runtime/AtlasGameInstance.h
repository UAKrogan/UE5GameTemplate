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
 * - Ensure subsystem exists
 */
UCLASS()
class GAME_API UAtlasGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;
};
