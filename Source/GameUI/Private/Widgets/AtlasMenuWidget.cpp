#include "Widgets/AtlasMenuWidget.h"

UAtlasMenuWidget::UAtlasMenuWidget()
{
	RequestedInputMode = EAtlasInputMode::Menu;
	bDismissOnBack = true;
	SetIsFocusable(true);
}
