#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/*
 * Common UI module: layered screen stack subsystem, base widgets,
 * screen registry data assets, and glyph/input-device services.
 */
class FGameUIModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
