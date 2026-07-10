// Fill out your copyright notice in the Description page of Project Settings.

#include "GameState/AtlasGameState.h"

#include "Components/GameFrameworkComponentManager.h"

AAtlasGameState::AAtlasGameState()
{
}

void AAtlasGameState::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AAtlasGameState::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(
		this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();

	InitializeGameState();
}

void AAtlasGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

void AAtlasGameState::InitializeGameState()
{
}
