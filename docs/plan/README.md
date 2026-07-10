# Planning Docs

The original design documents (00–13) drove the phased implementation (see the `Phase 0` … `Phase 11` commits on `master`) and were removed as their features landed — the implemented behavior is documented in [docs/guides/](../guides/README.md), which supersedes them. The full plan text remains available in git history.

## Open plans — genre support (phases 12–18)

Designs for evolving the template into a base for concrete genres (RTS, RPG with first/third person cameras). Not yet implemented.

- **[14_genre_gap_analysis.md](14_genre_gap_analysis.md)** — what RTS/RPG need vs what exists; framework-vs-feature-plugin split.
- **[15_camera_modes.md](15_camera_modes.md)** — camera mode stack with blending (FP/TP/top-down), config back-compat, ability-driven switching.
- **[16_interaction_teams.md](16_interaction_teams.md)** — interactable contract + detector component; `IGenericTeamAgentInterface` wiring with data-driven attitudes.
- **[17_selection_orders_rts.md](17_selection_orders_rts.md)** — selectable components, per-player selection subsystem, tag-based order pipeline into unit ASCs, RTS strategy pawn/controller.
- **[18_genre_roadmap.md](18_genre_roadmap.md)** — phased roadmap (12–18) including the remnants cleanup phase and sample RPG/RTS feature plugins.

## Smaller unimplemented remnants (scheduled into Phase 12)

- **Pre-travel acknowledgements** — `UAtlasLevelTransitionSubsystem` broadcasts `OnPreTravel` synchronously; the observer/ack pattern (wait for async work like an in-flight save before travel, with timeout) was designed but not needed yet.
- **`Atlas: Add Pawn Data` feature action** — registering an alternative `UAtlasPawnData` per game mode/context from a feature plugin.
- **`UAtlasSavableComponent::OwningFeatureTag`** — tagging savable actors with their owning feature (`Atlas.Feature.*`) so save data can be purged/ignored when a feature is deactivated.
- **Save/load completion delegates + Blueprint facade; stable component chunk IDs; automation tests** — carried from the save/load caveats.
- **Editor content** — MainMenu/Startup maps, widget Blueprints, and the sample plugins' data assets must be authored in the editor; see [docs/guides/setup.md](../guides/setup.md) and the sample plugin READMEs.
