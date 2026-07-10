#include "Widgets/AtlasNotificationWidget.h"

void UAtlasNotificationWidget::InitializeNotification(const FAtlasNotificationPayload& InPayload)
{
	Payload = InPayload;
	BP_OnNotificationInitialized(Payload);
}
