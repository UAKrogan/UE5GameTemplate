#pragma once

#include "Components/ActorComponent.h"
#include "Data/AtlasAbilitySet.h"
#include "AtlasPawnExtensionComponent.generated.h"

class UAtlasAbilitySystemComponent;
class UAtlasPawnData;

/*
 * Coordination hub for GAS initialization on a pawn.
 *
 * Holds the pawn data, reacts to possession changes, initializes the ASC
 * against the right owner (PlayerState for players, the pawn itself for AI)
 * and grants the pawn data's ability sets once on the authority.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class GAMEACTORS_API UAtlasPawnExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasPawnExtensionComponent();

	static UAtlasPawnExtensionComponent* FindPawnExtensionComponent(const APawn* Pawn);

	UFUNCTION(BlueprintCallable, Category = "Atlas|Pawn")
	void SetPawnData(const UAtlasPawnData* InPawnData);

	UFUNCTION(BlueprintCallable, Category = "Atlas|Pawn")
	const UAtlasPawnData* GetPawnData() const { return PawnData; }

	/*
	 * Called from the pawn's PossessedBy / UnPossessed / OnRep_Controller /
	 * OnRep_PlayerState. Initializes against a PlayerState-owned ASC when one
	 * is available; AI controllers call InitializeAbilitySystem directly with
	 * a pawn-owned ASC instead.
	 */
	void HandleControllerChanged();

	void InitializeAbilitySystem(UAtlasAbilitySystemComponent* InASC, AActor* InOwnerActor);
	void UninitializeAbilitySystem();

	UFUNCTION(BlueprintCallable, Category = "Atlas|Pawn")
	bool IsAbilitySystemReady() const { return AbilitySystemComponent != nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Atlas|Pawn")
	UAtlasAbilitySystemComponent* GetAtlasAbilitySystemComponent() const { return AbilitySystemComponent; }

	FSimpleMulticastDelegate OnAbilitySystemInitialized;
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

protected:
	//~UActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	/*
	 * Default pawn data; may be replaced at runtime via SetPawnData before
	 * the ability system initializes (e.g. by the game mode or a feature).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Atlas|Pawn")
	TObjectPtr<const UAtlasPawnData> PawnData;

private:
	APawn* GetOwningPawn() const;
	void GrantPawnDataAbilitySets();
	void RevokeGrantedAbilitySets();

	UPROPERTY(Transient)
	TObjectPtr<UAtlasAbilitySystemComponent> AbilitySystemComponent;

	TArray<FAtlasAbilitySetHandle> GrantedSetHandles;
};
