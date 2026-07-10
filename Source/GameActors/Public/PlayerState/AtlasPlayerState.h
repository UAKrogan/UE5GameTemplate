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

	UAtlasAbilitySystemComponent* GetAtlasAbilitySystemComponent() const { return AbilitySystemComponent; }

protected:
	virtual void BeginPlay() override;

	virtual void InitializePlayerState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atlas")
	TObjectPtr<UAtlasAbilitySystemComponent> AbilitySystemComponent;
};
