# 14 — Genre Gap Analysis (RTS / RPG First & Third Person)

Goal: evolve the Atlas template so a game project can build an **RTS** or an **RPG (first/third person)** on top of it without touching the source modules. This document identifies what each genre needs, what the template already provides, and what is missing.

The template rule stands: **no game mechanics in source modules.** Everything below is either genre-agnostic framework (source modules) or demonstration content (Game Feature plugins).

---

## What both genres need (and current coverage)

| Capability | RTS | RPG FP/TP | Current state |
|---|---|---|---|
| GAS actors, ability sets, input→ability tags | ✔ (unit abilities, commands) | ✔ (player abilities) | **Done** (Phases 5–6) |
| Screen stack / HUD / modals | ✔ | ✔ | **Done** (Phase 3) |
| Level flow, loading screens | ✔ | ✔ | **Done** (Phase 4) |
| Save/load with world snapshots | ✔ | ✔ | **Done** (+Phase 9 hardening) |
| Feature plugins for genre content | ✔ | ✔ | **Done** (Phase 7–8) |
| Camera **modes** with blending (FP ↔ TP ↔ top-down) | ✔ (strategy camera) | ✔ (FP/TP toggle, aim) | **Gap** — `UAtlasCameraConfig` applies static settings only; no mode stack, no blending, no FP support |
| Interaction with world objects (use/pickup/talk) | partial (capture points) | ✔ (core loop) | **Gap** — nothing exists |
| Teams / factions / attitude | ✔ (core) | ✔ (hostiles/friendlies) | **Gap** — `IGenericTeamAgentInterface` unused |
| Selection (single + marquee) | ✔ (core) | useful (lock-on targeting) | **Gap** — nothing exists |
| Orders/commands to non-possessed units | ✔ (core) | companion/pet orders | **Gap** — nothing exists |
| Cursor-driven gameplay input | ✔ (core) | inventory screens | **Partial** — input modes exist (`GameAndMenu`), but no click-to-world routing |
| Spectator/strategy pawn (pan/zoom/edge-scroll) | ✔ (core) | tactical pause view | **Gap** — only character/pawn/vehicle/mount bases |
| Inventory/equipment | shared econ | ✔ | **Deliberately out of source** — `UAtlasInventorySaveAdapter` stub persists items; the actual inventory is game/feature work |
| Dialogue, quests, fog of war, build systems | genre-specific | genre-specific | **Out of scope** — feature plugin / game project territory |

## Design conclusions

1. **Camera modes** are the highest-value shared gap: FP/TP switching for RPG and the top-down strategy camera for RTS are the same framework (a mode stack with blending, Lyra-style) with different mode assets. Extends the existing `UAtlasCameraExtensionComponent` rather than replacing it. → `15_camera_modes.md`
2. **Interaction + teams** are genre-agnostic actor-level services: an interactable contract with a detector component, and `IGenericTeamAgentInterface` wired into the existing controllers/player state. → `16_interaction_teams.md`
3. **Selection + orders** generalize beyond RTS: marquee selection is RTS-specific, but "selected set" + "issue tag-identified order that routes to a unit's ASC as a gameplay event" also covers RPG companion commands and lock-on targeting. The strategy pawn/controller shell completes the RTS player side. → `17_selection_orders_rts.md`
4. **Genre demos ship as feature plugins** (`GameFeature_SampleRPG`, `GameFeature_SampleRTS`) so both can be deleted without a trace. Inventory stays a sample concern.
5. Before new systems, the tracked **framework remnants** (save delegates + Blueprint facade, stable component IDs, pre-travel acks, Add Pawn Data action, OwningFeatureTag, automation tests) should land — genre work builds directly on those seams. → Phase 12 in `18_genre_roadmap.md`

## Dependency-graph impact

None. All new runtime classes fit the existing graph:
- Camera modes, interaction, teams, selection, orders, strategy pawn/controller → `GameActors` (actor-level gameplay framework; may use GAS + AIModule already in deps).
- New native tags (`Atlas.Camera.Mode.*`, `Atlas.Interaction.*`, `Atlas.Order.*`, `Atlas.Team.*`) → `GameCore` tags file, consistent with existing convention.
- UI for prompts/selection lives in game/feature widgets; source `GameUI` only gains generic hooks if needed (none anticipated — existing HUD element slots suffice).
