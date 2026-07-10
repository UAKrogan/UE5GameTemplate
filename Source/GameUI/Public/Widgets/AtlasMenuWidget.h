#pragma once

#include "Widgets/AtlasActivatableWidget.h"
#include "AtlasMenuWidget.generated.h"

/*
 * Base full-screen menu widget for the Menu layer (main menu, pause,
 * settings). Menus request UI-only input and pop on back navigation.
 */
UCLASS(Abstract, Blueprintable)
class GAMEUI_API UAtlasMenuWidget : public UAtlasActivatableWidget
{
	GENERATED_BODY()

public:
	UAtlasMenuWidget();
};
