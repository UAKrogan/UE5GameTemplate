// Fill out your copyright notice in the Description page of Project Settings.

#include "Runtime/AtlasGameInstance.h"
#include "Subsystems/AtlasGameFlowSubsystem.h"
#include "Subsystems/AtlasGameInstanceSubsystem.h"
#include "Logging/AtlasLogMacros.h"

void UAtlasGameInstance::Init()
{
	Super::Init();

	ATLAS_LOG(Log, "[GameInstance] Init");

	UAtlasGameInstanceSubsystem* Subsystem = GetSubsystem<UAtlasGameInstanceSubsystem>();

	check(Subsystem);

	/*
	 * Example usage (safe pattern):
	 *
	 * if (Subsystem)
	 * {
	 *     auto System = Subsystem->GetSystem<IMySystem>();
	 * }
	 */
}

void UAtlasGameInstance::OnStart()
{
	Super::OnStart();

	if (UAtlasGameFlowSubsystem* GameFlow = GetSubsystem<UAtlasGameFlowSubsystem>())
	{
		GameFlow->StartGameFlow();
	}
}

void UAtlasGameInstance::Shutdown()
{
	ATLAS_LOG(Log, "[GameInstance] Shutdown");

	Super::Shutdown();
}
