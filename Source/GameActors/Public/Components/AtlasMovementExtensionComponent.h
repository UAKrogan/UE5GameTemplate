#pragma once

#include "Components/ActorComponent.h"
#include "AtlasMovementExtensionComponent.generated.h"

/*
 * High-level movement mode, deliberately coarser than the engine's
 * EMovementMode: gameplay cares whether the pawn self-propels, rides, or
 * drives — not which physics path is active.
 */
UENUM(BlueprintType)
enum class EAtlasMovementMode : uint8
{
	// Self-propelled on foot (walking/falling).
	Ground,
	Swimming,
	Flying,
	// Passenger or driver of a vehicle; own movement component disabled.
	Vehicle,
	// Riding a mount; the mount's movement drives both.
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

	/*
	 * Switches the tracked mode and broadcasts OnMovementModeChanged.
	 * This component tracks state only — callers (vehicle/mount systems,
	 * game code) are responsible for driving the actual movement components.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas|Movement")
	void RequestMovementModeChange(EAtlasMovementMode NewMode);

	UPROPERTY(BlueprintAssignable, Category = "Atlas|Movement")
	FAtlasMovementModeChanged OnMovementModeChanged;

private:
	EAtlasMovementMode CurrentMode = EAtlasMovementMode::Ground;
};
