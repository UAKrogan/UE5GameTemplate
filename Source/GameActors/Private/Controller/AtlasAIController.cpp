// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/AtlasAIController.h"

#include "Components/AtlasAbilityExtensionComponent.h"
#include "Components/AtlasPawnExtensionComponent.h"
#include "GAS/AtlasAbilitySystemComponent.h"
#include "Logging/AtlasLogMacros.h"

AAtlasAIController::AAtlasAIController()
{
}

void AAtlasAIController::BeginPlay()
{
	Super::BeginPlay();

	InitializeAI();
}

void AAtlasAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UAtlasPawnExtensionComponent* PawnExt = UAtlasPawnExtensionComponent::FindPawnExtensionComponent(InPawn);
	UAtlasAbilityExtensionComponent* AbilityExt = UAtlasAbilityExtensionComponent::FindAbilityExtensionComponent(InPawn);
	if (PawnExt == nullptr || AbilityExt == nullptr)
	{
		return;
	}

	// AI pawns own their ASC; it lives and dies with the pawn.
	if (UAtlasAbilitySystemComponent* ASC = AbilityExt->GetOrCreateAbilitySystemComponent())
	{
		PawnExt->InitializeAbilitySystem(ASC, InPawn);
	}
}

void AAtlasAIController::OnUnPossess()
{
	if (UAtlasPawnExtensionComponent* PawnExt = UAtlasPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
	{
		PawnExt->UninitializeAbilitySystem();
	}

	Super::OnUnPossess();
}

void AAtlasAIController::InitializeAI()
{
}
