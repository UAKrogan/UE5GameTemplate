// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/AtlasCharacter.h"

#include "Components/AtlasAbilityExtensionComponent.h"
#include "Components/AtlasMovementExtensionComponent.h"
#include "Components/AtlasPawnExtensionComponent.h"
#include "GAS/AtlasAbilitySystemComponent.h"

AAtlasCharacter::AAtlasCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	PawnExtComp = CreateDefaultSubobject<UAtlasPawnExtensionComponent>(TEXT("PawnExtComp"));
	AbilityExtComp = CreateDefaultSubobject<UAtlasAbilityExtensionComponent>(TEXT("AbilityExtComp"));
	MovementExtComp = CreateDefaultSubobject<UAtlasMovementExtensionComponent>(TEXT("MovementExtComp"));
}

void AAtlasCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeCharacter();
}

UAbilitySystemComponent* AAtlasCharacter::GetAbilitySystemComponent() const
{
	return PawnExtComp->GetAtlasAbilitySystemComponent();
}

void AAtlasCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	PawnExtComp->HandleControllerChanged();
}

void AAtlasCharacter::UnPossessed()
{
	Super::UnPossessed();

	PawnExtComp->HandleControllerChanged();
}

void AAtlasCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtComp->HandleControllerChanged();
}

void AAtlasCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// The PlayerState (and its ASC) may replicate after the controller: this
	// is the client-side trigger for ability system initialization.
	PawnExtComp->HandleControllerChanged();
}

void AAtlasCharacter::InitializeCharacter()
{
}
