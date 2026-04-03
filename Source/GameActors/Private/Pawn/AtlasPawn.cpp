// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/AtlasPawn.h"

AAtlasPawn::AAtlasPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAtlasPawn::BeginPlay()
{
	Super::BeginPlay();

	InitializePawn();
}

void AAtlasPawn::InitializePawn()
{
}
