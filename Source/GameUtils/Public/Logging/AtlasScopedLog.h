#pragma once

#include "Logging/AtlasLogChannels.h"

/*
 * Atlas Scoped Log
 *
 * Purpose:
 * Automatically log the beginning and end of a scope.
 *
 * This is useful for:
 * - Lifecycle tracking
 * - Debugging initialization order
 * - Measuring execution flow
 *
 * How it works:
 * - Logs "Begin" when constructed
 * - Logs "End" when destroyed
 * - Uses RAII (stack-based lifetime)
 *
 * Important:
 * - The object must live on the stack
 * - Do NOT allocate dynamically (new/delete)
 *
 * Example usage:
 *
 *   void UAtlasGameInstanceSubsystem::Initialize(...)
 *   {
 *       FAtlasScopedLog Scope(TEXT("AtlasSubsystem::Initialize"));
 *
 *       // Your logic here
 *   }
 *
 * Output:
 *   [Scope Begin] AtlasSubsystem::Initialize
 *   ...
 *   [Scope End] AtlasSubsystem::Initialize
 */


class FAtlasScopedLog
{
public:
	/*
	 * Constructor
	 *
	 * @param InName - Name of the scope (function or system name)
	 */
	FAtlasScopedLog(const TCHAR* InName)
		: Name(InName)
	{
		UE_LOG(LogAtlas, Log, TEXT("[Scope Begin] %s"), Name);
	}

	/*
	 * Destructor
	 *
	 * Automatically called when the scope ends.
	 */
	~FAtlasScopedLog()
	{
		UE_LOG(LogAtlas, Log, TEXT("[Scope End] %s"), Name);
	}

private:
	/*
	 * Name of the tracked scope
	 */
	const TCHAR* Name;
};
