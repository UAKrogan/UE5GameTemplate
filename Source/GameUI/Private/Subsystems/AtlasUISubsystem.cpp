#include "Subsystems/AtlasUISubsystem.h"
#include "Logging/AtlasLogMacros.h"

void UAtlasUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ATLAS_LOG_UI(Log, "UISubsystem initialized");
}

void UAtlasUISubsystem::Deinitialize()
{
	ATLAS_LOG_UI(Log, "UISubsystem shutdown");

	Super::Deinitialize();
}

void UAtlasUISubsystem::OpenScreen(FName ScreenId)
{
	ATLAS_LOG_UI(Log, "OpenScreen: %s", *ScreenId.ToString());
}

void UAtlasUISubsystem::CloseScreen()
{
	ATLAS_LOG_UI(Log, "CloseScreen");
}
