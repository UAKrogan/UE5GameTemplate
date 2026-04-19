#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AtlasSavableComponent.generated.h"

class AActor;

UCLASS(ClassGroup = (Atlas), meta = (BlueprintSpawnableComponent))
class GAMECORE_API UAtlasSavableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasSavableComponent();

	virtual void OnRegister() override;
	virtual void PostLoad() override;
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;

	const FGuid& GetActorId() const;
	bool HasValidActorId() const;
	void EnsureActorId();

	static UAtlasSavableComponent* FindOnActor(const AActor* Actor);

private:
	void GenerateActorId();

	UPROPERTY(VisibleInstanceOnly, SaveGame, Category = "Savable")
	FGuid ActorId;
};
