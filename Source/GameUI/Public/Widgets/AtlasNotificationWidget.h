#pragma once

#include "Blueprint/UserWidget.h"
#include "Data/AtlasUITypes.h"
#include "AtlasNotificationWidget.generated.h"

/*
 * Toast-style transient notification for the Notification layer.
 *
 * Not activatable — notifications are non-blocking and never take input
 * focus. The UI subsystem removes the widget after the payload duration.
 */
UCLASS(Blueprintable)
class GAMEUI_API UAtlasNotificationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeNotification(const FAtlasNotificationPayload& InPayload);

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Notification")
	const FAtlasNotificationPayload& GetPayload() const { return Payload; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Atlas UI|Notification")
	void BP_OnNotificationInitialized(const FAtlasNotificationPayload& InPayload);

	UPROPERTY(BlueprintReadOnly, Category = "Atlas UI|Notification")
	FAtlasNotificationPayload Payload;
};
