// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/AtlasPlayerController.h"

#include "Components/AtlasInputExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Logging/AtlasLogMacros.h"

AAtlasPlayerController::AAtlasPlayerController()
{
	InputExtComp = CreateDefaultSubobject<UAtlasInputExtensionComponent>(TEXT("InputExtComp"));
}

void AAtlasPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AAtlasPlayerController::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(
		this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();

	InitializeController();
}

void AAtlasPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
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

	InputExtComp->Cleanup();

	Super::OnUnPossess();
}

void AAtlasPlayerController::InitializeController()
{
}

void AAtlasPlayerController::InitializeInput()
{
}
