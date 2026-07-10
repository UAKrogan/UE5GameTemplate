// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/AtlasPlayerController.h"

#include "Logging/AtlasLogMacros.h"

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

void AAtlasPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ATLAS_LOG_ACTORS(Log, "Player controller possessed %s", *GetNameSafe(InPawn));
}

void AAtlasPlayerController::OnUnPossess()
{
	ATLAS_LOG_ACTORS(Log, "Player controller unpossessed %s", *GetNameSafe(GetPawn()));

	Super::OnUnPossess();
}

void AAtlasPlayerController::InitializeController()
{
}

void AAtlasPlayerController::InitializeInput()
{
}
