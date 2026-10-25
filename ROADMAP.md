# ROADMAP

Loose plan, so it doesn't evaporate between sessions. Reorder freely.

## Done
- Milestone-based grid movement: head snaps to cell corners, `path` gains exactly one
  entry per crossing, body lerps between corners. Motion feels continuous.
- Unit-suffixed naming consistent everywhere (`pxUnitPos`, `nextPxTurnHead`, `cellPosOfHead`).
- `crossAndTurn()` extracted: one crossing body instead of four copies.
- raygui in the dev shell (header-only, `RAYGUI_IMPLEMENTATION` in `main.cpp`), no link flag.
- Settings shell: `<O>` opens it, `settingsOpen` freezes the simulation (accidental pause —
  decide if that's a feature), `GuiCheckBox` drives `debug`, save & exit button.
- Death + restart button/key; `is_oob` accounts for the unit-to-cell gap.

## Now
- Refine the settings/debug UI into a real in-game debugging tool
  (things I keep wanting to see or set while playing).

## Next — pick one, don't start two
- Small-bug pass over the existing `FIX:` comments
- Respawn: reinitialise *everything* from one place instead of re-deriving it by hand
- Apples: random spawn on a free cell, eating grows the snake

## Later
- Golden/super apple: temporary speed multiplier (x2 - x5, undecided)
- Global speed ramp: by length, by elapsed time, or both (undecided)
- Theme / colour sets
- Move `-`/`=` speed control out of live play into debug settings
- Playability pass so someone else can pick it up without a tour:
  no hidden settings, obvious controls on screen
- README: how to build (flake + `make`), controls, screenshots

## Someday
- Animations (eat, death, UI transitions)
- Toggleable 3D view: same board, rendered from a corner angle
  (dimetric / axonometric). Logic stays 2D; needs depth sorting.

## Decisions pending
- Speed boost: x2, x5, or something else? Duration? Stacking?
- Speed ramp: tied to length, time, or both?
- Pause: is `<O>` freezing the game intended, or should settings overlay live play?
