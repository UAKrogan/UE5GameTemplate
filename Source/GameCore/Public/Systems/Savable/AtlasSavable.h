#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Systems/Savable/AtlasSavableTypes.h"
#include "AtlasSavable.generated.h"

UINTERFACE()
class GAMECORE_API UAtlasSavable : public UInterface
{
	GENERATED_BODY()
};

/*
 * Implement on Actors or Components that want to participate in Atlas save/load.
 */
class GAMECORE_API IAtlasSavable
{
	GENERATED_BODY()

public:
	virtual void CaptureState(FAtlasSaveContext& Context) = 0;
	virtual void RestoreState(const FAtlasLoadContext& Context) = 0;
};
