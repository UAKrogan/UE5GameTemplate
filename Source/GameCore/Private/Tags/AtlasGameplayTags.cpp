#include "Tags/AtlasGameplayTags.h"

namespace AtlasGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameFlow_Startup, "Atlas.GameFlow.Startup", "Initial boot state before any map is playable.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameFlow_MainMenu, "Atlas.GameFlow.MainMenu", "Main menu is active.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameFlow_Loading, "Atlas.GameFlow.Loading", "A level transition is in progress.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameFlow_Gameplay, "Atlas.GameFlow.Gameplay", "Gameplay is active.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Game, "Atlas.UI.Layer.Game", "In-world/game layer beneath all UI.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_HUD, "Atlas.UI.Layer.HUD", "Always-visible HUD layer during gameplay.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Menu, "Atlas.UI.Layer.Menu", "Full-screen menu layer.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Modal, "Atlas.UI.Layer.Modal", "Modal/dialog layer above menus.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Notification, "Atlas.UI.Layer.Notification", "Toast/notification layer.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Loading, "Atlas.UI.Layer.Loading", "Topmost loading screen layer.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Native_Move, "Atlas.Input.Native.Move", "Native movement input (not routed through GAS).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Native_Look, "Atlas.Input.Native.Look", "Native look/camera input (not routed through GAS).");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Ability_Jump, "Atlas.Input.Ability.Jump", "Ability input binding for jump.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Save_Event_Manual, "Atlas.Save.Event.Manual", "Player-initiated save.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Save_Event_Autosave, "Atlas.Save.Event.Autosave", "Timer-driven autosave.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Save_Event_Checkpoint, "Atlas.Save.Event.Checkpoint", "Checkpoint save, e.g. before level travel.");
}
