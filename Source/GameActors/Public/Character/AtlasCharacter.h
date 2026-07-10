// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AtlasCharacter.generated.h"

class UAtlasAbilityExtensionComponent;
class UAtlasMovementExtensionComponent;
class UAtlasPawnExtensionComponent;

/*
 * Base humanoid character. Keeps ACharacter as the base (for
 * CharacterMovementComponent and native animation support) and mirrors the
 * AAtlasPawn extension component contract for GAS initialization.
 */
UCLASS()
class GAMEACTORS_API AAtlasCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAtlasCharacter();

	virtual void BeginPlay() override;

	//~IAbilitySystemInterface interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface interface

	//~APawn interface
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//~End of APawn interface

	UAtlasPawnExtensionComponent* GetPawnExtensionComponent() const { return PawnExtComp; }

protected:
	virtual void InitializeCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atlas")
	TObjectPtr<UAtlasPawnExtensionComponent> PawnExtComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atlas")
	TObjectPtr<UAtlasAbilityExtensionComponent> AbilityExtComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atlas")
	TObjectPtr<UAtlasMovementExtensionComponent> MovementExtComp;
};
