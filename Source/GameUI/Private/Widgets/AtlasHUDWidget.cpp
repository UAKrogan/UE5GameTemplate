#include "Widgets/AtlasHUDWidget.h"

#include "Logging/AtlasLogMacros.h"

UAtlasHUDWidget::UAtlasHUDWidget()
{
	RequestedInputMode = EAtlasInputMode::Game;
	bDismissOnBack = false;
}

void UAtlasHUDWidget::AddHUDElement(FName SlotName, UUserWidget* Element)
{
	if (!ATLAS_ENSURE(Element != nullptr))
	{
		return;
	}

	RemoveHUDElement(SlotName);

	HUDElements.Add(SlotName, Element);
	BP_OnHUDElementAdded(SlotName, Element);

	ATLAS_LOG_UI(Log, "HUD element added: %s", *SlotName.ToString());
}

void UAtlasHUDWidget::RemoveHUDElement(FName SlotName)
{
	TObjectPtr<UUserWidget> Existing;
	if (HUDElements.RemoveAndCopyValue(SlotName, Existing))
	{
		BP_OnHUDElementRemoved(SlotName, Existing);
		if (Existing != nullptr)
		{
			Existing->RemoveFromParent();
		}

		ATLAS_LOG_UI(Log, "HUD element removed: %s", *SlotName.ToString());
	}
}

UUserWidget* UAtlasHUDWidget::GetHUDElement(FName SlotName) const
{
	return HUDElements.FindRef(SlotName);
}
