#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/*
 * Runtime systems module: subsystem orchestration, game/level flow,
 * save/load pipeline, settings, asset manager, and native gameplay tags.
 */
class FGameCoreModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
