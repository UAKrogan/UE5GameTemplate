// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AtlasPlayerState.generated.h"

class UAtlasAbilitySystemComponent;

/*
 * Owns the ability system component for player-controlled pawns so abilities
 * and attributes survive pawn destruction and respawn (Lyra pattern). The
 * pawn is set as the avatar by UAtlasPawnExtensionComponent on possession.
 */
UCLASS()
class GAMEACTORS_API AAtlasPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAtlasPlayerState();

	//~IAbilitySystemInterface interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface interface

	/*
	 * Typed accessor for the player's ability system component.
	 */
	UAtlasAbilitySystemComponent* GetAtlasAbilitySystemComponent() const { return AbilitySystemComponent; }

protected:
	//~AActor interface (Modular Gameplay receiver)
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	/*
	 * Game-project hook called from BeginPlay. The base implementation sets
	 * the owner side of the ASC actor info; the avatar is assigned later on
	 * possession by the pawn extension component.
	 */
	virtual void InitializePlayerState();

	// Created as a default subobject so it replicates with the PlayerState.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atlas")
	TObjectPtr<UAtlasAbilitySystemComponent> AbilitySystemComponent;
};
