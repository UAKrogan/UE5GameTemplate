// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/AtlasPlayerController.h"

AAtlasPlayerController::AAtlasPlayerController()
{
}

void AAtlasPlayerController::BeginPlay()
{
	Super::BeginPlay();

	InitializeController();
}

void AAtlasPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InitializeInput();
}

void AAtlasPlayerController::InitializeController()
{
}

void AAtlasPlayerController::InitializeInput()
{
}
