// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/AtlasPawn.h"

#include "Components/AtlasAbilityExtensionComponent.h"
#include "Components/AtlasInputExtensionComponent.h"
#include "Components/AtlasPawnExtensionComponent.h"
#include "Controller/AtlasPlayerController.h"
#include "GAS/AtlasAbilitySystemComponent.h"

AAtlasPawn::AAtlasPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	PawnExtComp = CreateDefaultSubobject<UAtlasPawnExtensionComponent>(TEXT("PawnExtComp"));
	AbilityExtComp = CreateDefaultSubobject<UAtlasAbilityExtensionComponent>(TEXT("AbilityExtComp"));
}

void AAtlasPawn::BeginPlay()
{
	Super::BeginPlay();

	InitializePawn();
}

UAbilitySystemComponent* AAtlasPawn::GetAbilitySystemComponent() const
{
	return PawnExtComp->GetAtlasAbilitySystemComponent();
}

void AAtlasPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	PawnExtComp->HandleControllerChanged();
}

void AAtlasPawn::UnPossessed()
{
	Super::UnPossessed();

	PawnExtComp->HandleControllerChanged();
}

void AAtlasPawn::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtComp->HandleControllerChanged();
}

void AAtlasPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// The PlayerState (and its ASC) may replicate after the controller: this
	// is the client-side trigger for ability system initialization.
	PawnExtComp->HandleControllerChanged();
}

void AAtlasPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Runs on the owning client (unlike OnPossess), so this is where pawn
	// data input configs are applied and ability actions bound.
	if (const AAtlasPlayerController* PC = Cast<AAtlasPlayerController>(GetController()))
	{
		PC->GetInputExtensionComponent()->InitializeForPawn(this, PlayerInputComponent);
	}
}

void AAtlasPawn::InitializePawn()
{
}
