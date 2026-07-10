// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "AtlasGameMode.generated.h"

class UAtlasPawnData;

/*
 * Base game mode for Atlas projects.
 *
 * Extends AGameMode (rather than AGameModeBase) to provide match state
 * support (WaitingToStart, InProgress, WaitingPostMatch) required by any
 * non-trivial game loop.
 */
UCLASS()
class GAMEACTORS_API AAtlasGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AAtlasGameMode();

	//~AActor interface (Modular Gameplay receiver)
	virtual void PreInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	virtual void StartPlay() override;

	//~AGameModeBase interface

	/*
	 * Resolves the pawn class from DefaultPawnData when set; falls back to
	 * the standard DefaultPawnClass otherwise.
	 */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	//~End of AGameModeBase interface

protected:
	/*
	 * Game-project hook called from StartPlay after the framework setup.
	 */
	virtual void InitializeGameMode();

	/*
	 * Per-map pawn configuration; when set, the default pawn class is
	 * resolved from this pawn data instead of DefaultPawnClass.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atlas")
	TSoftObjectPtr<UAtlasPawnData> DefaultPawnData;
};
