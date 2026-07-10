#pragma once

#include "Components/ActorComponent.h"
#include "AtlasMovementExtensionComponent.generated.h"

UENUM(BlueprintType)
enum class EAtlasMovementMode : uint8
{
	Ground,
	Swimming,
	Flying,
	Vehicle,
	Mounted
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAtlasMovementModeChanged, EAtlasMovementMode, OldMode, EAtlasMovementMode, NewMode);

/*
 * Tracks the pawn's high-level movement mode (ground, vehicle, mounted, ...)
 * so gameplay and animation can react to transitions without knowing about
 * the specific movement component driving them.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class GAMEACTORS_API UAtlasMovementExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasMovementExtensionComponent();

	static UAtlasMovementExtensionComponent* FindMovementExtensionComponent(const APawn* Pawn);

	UFUNCTION(BlueprintCallable, Category = "Atlas|Movement")
	EAtlasMovementMode GetCurrentMovementMode() const { return CurrentMode; }

	UFUNCTION(BlueprintCallable, Category = "Atlas|Movement")
	void RequestMovementModeChange(EAtlasMovementMode NewMode);

	UPROPERTY(BlueprintAssignable, Category = "Atlas|Movement")
	FAtlasMovementModeChanged OnMovementModeChanged;

private:
	EAtlasMovementMode CurrentMode = EAtlasMovementMode::Ground;
};
