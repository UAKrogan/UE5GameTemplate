#include "Widgets/AtlasModalWidget.h"

UAtlasModalWidget::UAtlasModalWidget()
{
	RequestedInputMode = EAtlasInputMode::Menu;
	bDismissOnBack = true;
	SetIsFocusable(true);
}

void UAtlasModalWidget::SetPayload(const FAtlasModalPayload& InPayload, FAtlasModalResultDelegate InResultDelegate)
{
	Payload = InPayload;
	ResultDelegate = MoveTemp(InResultDelegate);
	bResultSent = false;

	BP_OnPayloadSet(Payload);
}

void UAtlasModalWidget::Confirm()
{
	FinishModal(true);
}

void UAtlasModalWidget::Cancel()
{
	FinishModal(false);
}

void UAtlasModalWidget::FinishModal(bool bConfirmed)
{
	// Guard against double-fire from a button click racing back-navigation.
	if (bResultSent)
	{
		return;
	}
	bResultSent = true;

	ResultDelegate.ExecuteIfBound(bConfirmed);
	DeactivateWidget();
}
