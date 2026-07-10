# Planning Docs

The original design documents (00–13) drove the phased implementation (see the `Phase 0` … `Phase 11` commits on `master`) and have been removed as their features landed — the implemented behavior is documented in [docs/guides/](../guides/README.md), which supersedes them. The full plan text remains available in git history.

All numbered plans are now implemented, including the movement architecture (`06_movement.md`, implemented as Phase 11: vehicle/mount pawns and extension components, movement-mode routing with replication, camera configs, and input-context switching).

## Smaller unimplemented remnants

- **Pre-travel acknowledgements** — `UAtlasLevelTransitionSubsystem` broadcasts `OnPreTravel` synchronously; the observer/ack pattern (wait for async work like an in-flight save before travel, with timeout) was designed but not needed yet.
- **`Atlas: Add Pawn Data` feature action** — registering an alternative `UAtlasPawnData` per game mode/context from a feature plugin.
- **`UAtlasSavableComponent::OwningFeatureTag`** — tagging savable actors with their owning feature (`Atlas.Feature.*`) so save data can be purged/ignored when a feature is deactivated.
- **Editor content** — MainMenu/Startup maps, widget Blueprints, and the sample plugins' data assets must be authored in the editor; see [docs/guides/setup.md](../guides/setup.md) and the sample plugin READMEs.
