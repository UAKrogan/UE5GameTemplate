// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerState/AtlasPlayerState.h"

#include "Components/GameFrameworkComponentManager.h"
#include "GAS/AtlasAbilitySystemComponent.h"

AAtlasPlayerState::AAtlasPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAtlasAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// GAS state changes (attributes, tags) need a higher frequency than the
	// PlayerState default, which is tuned for score-board style data.
	SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* AAtlasPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAtlasPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AAtlasPlayerState::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(
		this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();

	InitializePlayerState();
}

void AAtlasPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

void AAtlasPlayerState::InitializePlayerState()
{
	// Owner-side actor info; the avatar is assigned on possession by the
	// pawn extension component.
	if (AbilitySystemComponent != nullptr)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, nullptr);
	}
}
