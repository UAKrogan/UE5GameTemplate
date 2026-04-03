// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AtlasPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GAMEACTORS_API AAtlasPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AAtlasPlayerState();

protected:
	virtual void BeginPlay() override;

	virtual void InitializePlayerState();
};
