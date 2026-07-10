// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMode/AtlasGameMode.h"

#include "Data/AtlasPawnData.h"

AAtlasGameMode::AAtlasGameMode()
{
}

void AAtlasGameMode::StartPlay()
{
	Super::StartPlay();

	InitializeGameMode();
}

UClass* AAtlasGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UAtlasPawnData* PawnData = DefaultPawnData.LoadSynchronous())
	{
		if (UClass* PawnClass = PawnData->PawnClass.LoadSynchronous())
		{
			return PawnClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AAtlasGameMode::InitializeGameMode()
{
}
