#include "Widgets/AtlasActivatableWidget.h"

#include "Logging/AtlasLogMacros.h"

void UAtlasActivatableWidget::InitializeWithContext(UObject* Context)
{
	BP_OnInitializedWithContext(Context);
}

EAtlasInputMode UAtlasActivatableWidget::GetRequestedInputMode() const
{
	return RequestedInputMode;
}

bool UAtlasActivatableWidget::GetRequestsDismissOnBack() const
{
	return bDismissOnBack;
}

void UAtlasActivatableWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	ATLAS_LOG_UI(Verbose, "Screen activated: %s", *GetName());
}

void UAtlasActivatableWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	ATLAS_LOG_UI(Verbose, "Screen deactivated: %s", *GetName());
}
