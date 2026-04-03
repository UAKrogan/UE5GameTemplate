// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/AtlasCharacter.h"
#include "Logging/AtlasLogMacros.h"

AAtlasCharacter::AAtlasCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAtlasCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeCharacter();
}

void AAtlasCharacter::InitializeCharacter()
{
}

void AAtlasCharacter::Save(FArchive& Ar)
{
	ATLAS_LOG_ACTORS(Log, "Save called on %s", *GetName());
}

void AAtlasCharacter::Load(FArchive& Ar)
{
	ATLAS_LOG_ACTORS(Log, "Load called on %s", *GetName());
}
