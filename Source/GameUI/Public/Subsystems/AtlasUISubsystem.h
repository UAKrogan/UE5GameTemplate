#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "AtlasUISubsystem.generated.h"

/*
 * UI Subsystem
 *
 * Responsible for:
 * - Opening screens
 * - Managing UI state
 */
UCLASS()
class GAMEUI_API UAtlasUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void OpenScreen(FName ScreenId);
	void CloseScreen();
};
