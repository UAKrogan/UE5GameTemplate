#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/*
 * Primary game module. Thin host that assembles the Atlas framework
 * modules; contains no reusable logic beyond the game instance bootstrap.
 */
class FGameModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
