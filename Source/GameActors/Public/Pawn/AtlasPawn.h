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

	//~AActor interface (Modular Gameplay receiver)
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~IAbilitySystemInterface interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface interface

	//~APawn interface
	// Possession hooks all route to PawnExtComp->HandleControllerChanged();
	// the OnRep_* overrides are the client-side triggers (server-only
	// OnPossess never runs on clients).
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;

	// Runs on the owning client; forwards to the controller's input
	// extension component to apply pawn data input configs.
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//~End of APawn interface

	UAtlasPawnExtensionComponent* GetPawnExtensionComponent() const { return PawnExtComp; }

protected:
	/*
	 * Game-project hook called from BeginPlay after the framework setup.
	 */
	virtual void InitializePawn();

	// GAS initialization coordinator (pawn data, possession, ASC binding).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atlas")
	TObjectPtr<UAtlasPawnExtensionComponent> PawnExtComp;

	// Provides a pawn-owned ASC when this pawn is AI/standalone controlled.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atlas")
	TObjectPtr<UAtlasAbilityExtensionComponent> AbilityExtComp;
};
