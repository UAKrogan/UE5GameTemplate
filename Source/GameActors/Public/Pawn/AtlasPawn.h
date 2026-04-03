// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AtlasPawn.generated.h"

UCLASS()
class GAMEACTORS_API AAtlasPawn : public APawn
{
	GENERATED_BODY()

public:
	AAtlasPawn();

	virtual void BeginPlay() override;

protected:
	virtual void InitializePawn();
};
