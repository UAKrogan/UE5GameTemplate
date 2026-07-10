// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AtlasPawn.generated.h"

class UAtlasAbilityExtensionComponent;
class UAtlasPawnExtensionComponent;

/*
 * Base for all Atlas pawns. GAS-ready via the extension component contract:
 * possession changes route through UAtlasPawnExtensionComponent, which
 * initializes the ability system against the right owner (PlayerState for
 * players, the pawn itself for AI).
 */
UCLASS()
class GAMEACTORS_API AAtlasPawn : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAtlasPawn();

	virtual void BeginPlay() override;

	//~IAbilitySystemInterface interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface interface

	//~APawn interface
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;
	//~End of APawn interface

	UAtlasPawnExtensionComponent* GetPawnExtensionComponent() const { return PawnExtComp; }

protected:
	virtual void InitializePawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atlas")
	TObjectPtr<UAtlasPawnExtensionComponent> PawnExtComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atlas")
	TObjectPtr<UAtlasAbilityExtensionComponent> AbilityExtComp;
};
