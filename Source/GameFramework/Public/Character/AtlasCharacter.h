// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AtlasCharacter.generated.h"

UCLASS()
class GAMEFRAMEWORK_API AAtlasCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAtlasCharacter();

	virtual void BeginPlay() override;

protected:
	virtual void InitializeCharacter();
};
