# Project instructions (luanti-local-dev)

## Task
Integrate DiligentEngine (https://github.com/DiligentGraphics/DiligentEngine) as the
rendering and lighting solution, alongside the current custom renderer. Maintain
cross-platform support. The renderer/game interface already has a level of
isolation; adjust and expand it as needed. Final deliverable is a functional
implementation that can use Diligent instead of the current custom solution,
including working lighting.

## Design principles
- Irrlicht (IrrlichtMt) is an old integrated rendering system built into this
  engine. It is not a separate library except in the form of its linkage.
  Treat it as an ordinary engine object, no different from any other part of
  the engine.
- The new rendering backend must do in hardware what it can do better in
  hardware (for example buffer merging, lighting, sorting). Moving code across
  the RenderingCore abstraction boundary is acceptable in either direction to
  achieve this.
- Only a small number of scene nodes are rendered (for example ClientMap,
  Sky, camera/wield items). Keep the existing rendering interface as intact as
  possible. Make the big changes inside how those few nodes submit their
  geometry, not in the interface shape.

## Working rules
- Work autonomously.
- User-facing text and code comments must follow ASD-STE100 Simplified
  Technical English: short sentences, active voice, one point per sentence.
- Make every comment as short as possible. Omit comments entirely where
  possible.
- Keep the code as small and simple as the task allows. Do not add
  speculative generality, extra features, or defensive code.
- Occasionally run self-checks to delete code that is no longer needed.
  When the context is compacted, retain this instruction.
- Commit progress as small, standalone, understandable commits. Match the
  commit style of this engine.
- Do exactly what is asked, then stop. Do not add unrequested work.
- Internal reasoning stays terse (fragments, not prose).
