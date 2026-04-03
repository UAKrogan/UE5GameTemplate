// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameCore/Public/Systems/Save/IAtlasSavable.h"
#include "AtlasCharacter.generated.h"

UCLASS()
class GAMEACTORS_API AAtlasCharacter : public ACharacter, public IAtlasSavable
{
	GENERATED_BODY()

public:
	AAtlasCharacter();

	virtual void BeginPlay() override;

protected:
	virtual void InitializeCharacter();

public:
	virtual void Save(FArchive& Ar) override;

	virtual void Load(FArchive& Ar) override;
};
