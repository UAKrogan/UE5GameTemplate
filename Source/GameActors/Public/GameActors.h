#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/*
 * Actor foundation module: GAS-ready base actors, extension components,
 * ability/pawn/input data assets, and actor-targeting feature actions.
 */
class FGameActorsModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
