#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/*
 * Foundation module: log categories, logging macros, and assertion
 * helpers shared by every other Atlas module. No game logic.
 */
class FGameUtilsModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
