#pragma once

#include "NativeGameplayTags.h"

/*
 * Native gameplay tags owned by the Atlas framework.
 *
 * Tags are registered automatically at module startup via the
 * UE_DEFINE_GAMEPLAY_TAG machinery in AtlasGameplayTags.cpp — no manual
 * registration step is required.
 *
 * Namespace convention (see docs/guides/conventions.md):
 *   Atlas.Input.*      — input-related tags
 *   Atlas.Ability.*    — ability-related tags
 *   Atlas.UI.*         — UI screen/layer tags
 *   Atlas.GameFlow.*   — game state flow tags
 *   Atlas.Save.*       — save/load event tags
 *   Atlas.Feature.*    — feature activation tags
 *
 * Game Feature plugins declare their own tags in their own modules following
 * the same convention.
 */
namespace AtlasGameplayTags
{
	// Game flow states
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameFlow_Startup);
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameFlow_MainMenu);
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameFlow_Loading);
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameFlow_Gameplay);

	// UI layers
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Game);
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_HUD);
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Menu);
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Modal);
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Notification);
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Loading);

	// Native (non-ability) input
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Move);
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Look);

	// Ability input
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Jump);

	// Save/load events
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Save_Event_Manual);
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Save_Event_Autosave);
	GAMECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Save_Event_Checkpoint);
}
