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

	void SetPayload(const FAtlasModalPayload& InPayload, FAtlasModalResultDelegate InResultDelegate);

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Modal")
	void Confirm();

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
