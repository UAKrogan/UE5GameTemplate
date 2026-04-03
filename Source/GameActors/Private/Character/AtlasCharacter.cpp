// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/AtlasCharacter.h"

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
