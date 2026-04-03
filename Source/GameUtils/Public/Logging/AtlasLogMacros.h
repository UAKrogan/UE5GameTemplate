#pragma once

#include "Logging/AtlasLogChannels.h"

/*
 * Atlas Logging Macros
 *
 * Purpose:
 * Provide a unified, consistent, and maintainable logging interface
 * across all Atlas modules.
 *
 * Why use macros instead of UE_LOG directly:
 * - Enforces consistent formatting
 * - Reduces boilerplate
 * - Allows future changes in one place
 * - Enables compile-time stripping or redirection
 *
 * Usage example:
 *   ATLAS_LOG_CORE(Log, "Subsystem initialized");
 *   ATLAS_LOG_CORE(Warning, "Save slot not found: %s", *SlotName);
 */


/* ============================================================================
 * Base logging macro (generic usage)
 * ============================================================================
 *
 * Use this when logging something not tied to a specific module.
 *
 * Example:
 *   ATLAS_LOG(Log, "Application started");
 */
#define ATLAS_LOG(Verbosity, Format, ...) \
	UE_LOG(LogAtlas, Verbosity, TEXT(Format), ##__VA_ARGS__)


/* ============================================================================
 * Module-specific logging macros
 * ============================================================================
 *
 * These macros automatically prepend a module tag to messages.
 * This improves readability in large logs.
 *
 * Recommended usage:
 * - Always prefer module-specific macros over ATLAS_LOG
 */


/*
 * Actors module logging
 *
 * Used for:
 * - Gameplay actors
 * - Controllers
 * - Gameplay framework lifecycle
 *
 * Example:
 *   ATLAS_LOG_ACTORS(Log, "Player controller initialized");
 */
#define ATLAS_LOG_ACTORS(Verbosity, Format, ...) \
	UE_LOG(LogAtlasActors, Verbosity, TEXT("[Actors] " Format), ##__VA_ARGS__)


/*
 * Core module logging
 *
 * Used for:
 * - Runtime systems
 * - Subsystems
 * - Core orchestration
 *
 * Example:
 *   ATLAS_LOG_CORE(Warning, "Save system missing: %s", *SlotName);
 */
#define ATLAS_LOG_CORE(Verbosity, Format, ...) \
	UE_LOG(LogAtlasCore, Verbosity, TEXT("[Core] " Format), ##__VA_ARGS__)


/*
 * UI module logging
 *
 * Used for:
 * - UI navigation
 * - Screen changes
 * - Input/UI interaction
 *
 * Example:
 *   ATLAS_LOG_UI(Log, "Opened screen: %s", *ScreenName);
 */
#define ATLAS_LOG_UI(Verbosity, Format, ...) \
	UE_LOG(LogAtlasUI, Verbosity, TEXT("[UI] " Format), ##__VA_ARGS__)


/*
 * Utils module logging
 *
 * Used for:
 * - Helper functions
 * - Utility operations
 * - Low-level shared logic
 *
 * Example:
 *   ATLAS_LOG_UTILS(Log, "Normalized string: %s", *Result);
 */
#define ATLAS_LOG_UTILS(Verbosity, Format, ...) \
	UE_LOG(LogAtlasUtils, Verbosity, TEXT("[Utils] " Format), ##__VA_ARGS__)


/* ============================================================================
 * Assertions and runtime checks
 * ============================================================================
 *
 * These macros wrap Unreal's check/ensure with consistent usage.
 */


/*
 * Hard assertion (crashes in debug if condition fails)
 *
 * Use when:
 * - Condition must never fail
 * - Indicates programmer error
 *
 * Example:
 *   ATLAS_CHECK(PlayerController != nullptr);
 */
#define ATLAS_CHECK(Expr) \
	check(Expr)


/*
 * Soft assertion (logs error but does not crash)
 *
 * Use when:
 * - You want to detect invalid state
 * - Execution can continue safely
 *
 * Example:
 *   ATLAS_ENSURE(SaveSystem != nullptr);
 */
#define ATLAS_ENSURE(Expr) \
	ensure(Expr)


/*
 * Soft assertion with custom message
 *
 * Logs additional context if condition fails.
 *
 * Example:
 *   ATLAS_ENSURE_MSG(SaveSystem != nullptr, "SaveSystem missing in %s", *GetName());
 */
#define ATLAS_ENSURE_MSG(Expr, Format, ...) \
	if (!ensure(Expr)) \
	{ \
		UE_LOG(LogAtlas, Error, TEXT("[Ensure] " Format), ##__VA_ARGS__); \
	}


/* ============================================================================
 * Debug-only logging
 * ============================================================================
 *
 * These logs are compiled out in Shipping builds.
 *
 * Use for:
 * - Temporary debug output
 * - High-frequency logs
 */

#if !UE_BUILD_SHIPPING

#define ATLAS_LOG_DEBUG(Category, Format, ...) \
	UE_LOG(Category, Log, TEXT("[Debug] " Format), ##__VA_ARGS__)

#else

#define ATLAS_LOG_DEBUG(Category, Format, ...)

#endif
