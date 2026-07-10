#pragma once

#include "Widgets/AtlasActivatableWidget.h"
#include "AtlasModalWidget.generated.h"

/*
 * Base modal dialog widget for the Modal layer.
 *
 * The UI subsystem sets the payload and result callback after pushing; a
 * Blueprint subclass binds its buttons to Confirm()/Cancel().
 */
UCLASS(Abstract, Blueprintable)
class GAMEUI_API UAtlasModalWidget : public UAtlasActivatableWidget
{
	GENERATED_BODY()

public:
	UAtlasModalWidget();

	/*
	 * Called by the UI subsystem right after the push. Stores the payload,
	 * arms the result delegate, and fires BP_OnPayloadSet for text binding.
	 */
	void SetPayload(const FAtlasModalPayload& InPayload, FAtlasModalResultDelegate InResultDelegate);

	/*
	 * Resolves the dialog with bConfirmed = true and deactivates. Bind the
	 * confirm button to this in the widget Blueprint.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Modal")
	void Confirm();

	/*
	 * Resolves the dialog with bConfirmed = false and deactivates. Also used
	 * by DismissModal and back navigation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Modal")
	void Cancel();

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Modal")
	const FAtlasModalPayload& GetPayload() const { return Payload; }

protected:
	/*
	 * Blueprint hook fired once the payload is available, for text binding.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Atlas UI|Modal")
	void BP_OnPayloadSet(const FAtlasModalPayload& InPayload);

	UPROPERTY(BlueprintReadOnly, Category = "Atlas UI|Modal")
	FAtlasModalPayload Payload;

private:
	void FinishModal(bool bConfirmed);

	FAtlasModalResultDelegate ResultDelegate;
	bool bResultSent = false;
};
