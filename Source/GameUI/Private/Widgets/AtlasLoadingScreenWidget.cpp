#include "Widgets/AtlasLoadingScreenWidget.h"

void UAtlasLoadingScreenWidget::InitializeLoadingScreen(const FAtlasLoadingScreenConfig& InConfig)
{
	Config = InConfig;
	Progress = 0.0f;

	BP_OnLoadingScreenInitialized(Config);
}

void UAtlasLoadingScreenWidget::SetProgress(float InProgress)
{
	Progress = FMath::Clamp(InProgress, 0.0f, 1.0f);
	BP_OnProgressChanged(Progress);
}
