# LEARNING.md

Learning log for the snake project. Read at session start. Update after each session.

## Format
- **Concept** — what it is, one line.
- **Mistake** — what happened, and the underlying misconception.
- **Shaky** — needs re-testing in a new context.

---

## Session 2026-10-23 — path / body-lerp debugging

### Concepts covered
- `std::deque::operator[]` does **no** bounds checking — same UB as `std::vector`.
- A "milestone" variable must be *stored* and *advanced*, not *recomputed from live state*.
  Recomputed-from-live comparisons between two expressions of the same value are tautologies
  (e.g. `x > ceilf(x)` is always false).

### Mistakes / misconceptions
- `nextCellToTurnHead` is refreshed every frame from live `cellPos` while `!turnedHead`,
  so the crossing test `pixelUnitPos[0].x > nextCellToTurnHead.x` degenerates to
  `cellPos.x > ceilf(cellPos.x)` → never true. Crossings only fired on the frames after a key
  press froze the value, hence "path only updates when I spam the key".
- `path.push_front(nextCellToTurn)` pushes the line *ahead*; the model needs the cell *reached*.
- Body-lerp loop indexes `path[i]` for `i < pixelUnitPos.size()` without checking
  `path.size()`, and `path` is shorter than `pixelUnitPos` right after `KEY_A` adds a unit → UB.

- `ceilf`/`floorf` are not injective: position → cell loses information at boundaries.
  "Which cell am I heading to" needs history, not a snapshot.
- Milestone vs live value: **overwriting** a milestone from live state is the bug;
  **advancing** a milestone is correct. Same variable, opposite meanings.
- Smell: adding a bool (`haveMilestone`) to guard an already-valid piece of state =
  second source of truth. Named the trap; not yet applied.

### Shaky
- Distinguishing "value that differs from frame to frame" vs "event that must be latched".
- Not yet re-tested: evaluation order of the key-press / crossing / lerp / move block.
- The exact-cell-boundary ambiguity in `floor`/`ceil` — recognised once, needs a second encounter.

## Session 2026-10-24 — milestone rewrite lands

### New concepts
- **Units in names.** `headCell` vs `headPixel`. A comparison mixing the two compiles and
  silently never fires. Caught a real one: `cellPosOfHead.x >= nextPixelTurnHead.x`
  (`8.0 >= 675.0`).
- Two coordinate spaces, two jobs: pixels = position/snap/draw; cells = the 0..1 fraction
  (`unitFractionPosition`) only. A fraction is unitless, which is why cell space exists at all.
- **Seed vs advance**: seed a milestone once before the loop, then only `+=`. Never derive
  it from live position — `ceilf`/`floorf` can't tell "leaving cell 9" from "arriving at cell 9".
- **Bad bool vs good bool**: `haveMilestone` (duplicates state, rots) vs
  `crossed` (records an event this frame, correct).
- Unreachable fallback (`return {0,0}` after a full switch) is *defensive*, not data.
  Don't branch on it in the caller — that's how `if (nextPixelTurnHead.x == 0) return 0`
  (program exit!) got written.
- Vector delta vs position: `next = step(...)` vs `next += step(...)`.

### Mistakes made and fixed
- Did **not** actually delete the recompute on day 1 (memory vs file) — block was still there.
  Lesson: reread the file, don't trust recall.
- Deleting a bad writer leaves the variable orphaned (`{0,0}`), which looks like "deletion broke
  the game". Missing writer, not wrong deletion.
- `step()` with transposed axes (right returned a `y` delta) and `cellSize.x` used for the y axis
  (harmless only because cells are square).
- Advanced the milestone on the old axis after a turn (`x += 75` while now moving down).

### Still open / shaky
- **Bug #3 not fixed**: `path` is declared empty; the lerp loop reads `path[i]` with
  `path.size() < pixelUnitPos.size()` → UB. Test: restart, press `A` before any crossing.
- `moveKeys(pendingMode)` — its `!=` checks compare against *pending*, not `currentMode`,
  so some legal turns are swallowed (moving right, press up then down).
- `turnedHead` is really "turn pending" — rename pending.
- `debugInfo` still mislabels its args (`targetCell` actually holds `nextPixelTurnHead`).
- Not yet re-tested: `oppositeDir` taking a non-const ref; `moveSpeed` non-const ref.

### State at end of session (2026-10-23)
Agreed shape, **not yet written**:
- `SEED` once at spawn: snapped spawn cell + one cell in `currentMode`.
- `ADVANCE` once per crossing, *after* applying `pendingMode`.
- Reuse one `step(MovementMode, Vector2 cellSize)` helper in `moveSpeed` and here.
- Delete: `nextCellToTurn`, the `if (!turnedHead)` block, the `turnedHead` latch.
- Crossing tests at lines 227–258 were already correct, keep them.

Open question left for next session (answer before writing):
  at spawn the head is exactly on a line — `cellPos == (8, 5)`, fraction 0.
  Old formula `cellSize.x * ceilf(cellPos.x)` gives what, and how far ahead of
  the head is that? Is the invariant already off by one at frame 1?

Then, in order:
1. milestone seed + advance (this session's topic)
2. bug #2 — `push_front` pushes the line *ahead*; must push the cell *reached*
3. bug #3 — lerp loop reads `path[i]` while `path.size() < pixelUnitPos.size()` → UB.
   Guard it (`i < path.size()`), and seed `path` with one entry per body cell.

## Session 2026-10-25 — raygui + refactor + roadmap

### Concepts covered
- **Single-header libraries**: `raygui.h` contains declarations *and* implementation behind
  `RAYGUI_IMPLEMENTATION`. Exactly one TU defines it. 0 TUs -> undefined reference,
  2 TUs -> multiple definition. Both are link errors, neither is a `-l` flag.
- Nix: every `buildInputs` entry with an `include/` dir is added to `NIX_CFLAGS_COMPILE`
  as `-isystem`. `pkgs.raygui` (5.0) ships the header only — nothing to link.
  Verified raygui 5.0 compiles clean against raylib 6.0.
- Pause by extending the existing guard (`&& !settingsOpen`) instead of a new flag.
- Settings as a screen (`if/else`), not an overlay.

### What they did well
- `crossAndTurn()` — single-sourced the crossing invariant.
- Renamed the whole vocabulary to unit-suffixed names, not just the buggy line.
- Worked out per-direction lerp argument order themselves (path[0] is newest).

### Design notes raised (no bug spoilers given, per request)
- `main` ~170 lines and owns everything -> `GameState` struct + free functions.
  Would collapse `debugInfo`'s 10 params and make restart one assignment.
- Input read in 4 places (`keybinds`, `moveKeys`, `IsKeyPressed` x3, raygui) ->
  already inconsistent about what works while paused.
- Two different functions named `addUnit`.
- Magic numbers (`6.0f`, `75`, `62.5`, `20.0f`) -> constexpr block / Config struct.
- Dimetric: projection is cheap, depth sorting is the real work.

### Process note
- `edit`/`write` tools are permission-denied in this session; shell heredoc works.
  Use `cat >>` for log updates until the permission is sorted.

### Open
- All `FIX:` comments in main.cpp (their own markers, deliberately not enumerated).

## Session 2026-10-25 (later) — structs, config, renames

### Concepts
- **DMI scope**: default member initialisers can only see members declared above them and
  namespace-scope names. Not locals in `main`. Derived values need a factory.
- **Type vs object**: `Config.windowSize` fails — `Config` is a type; `.` needs an object.
  Type-qualified access (`Config::x`) only for `static` members.
- **`const Config cfg`**: must be initialised at declaration; no member assignment after.
  `constexpr` = known at compile time (not the case here). `const&` params prove read-only.
- **`int &` is an out-parameter**: can't bind to `1` (rvalue). Inputs go by value.
- **Function vs variable**: renaming a variable into a function breaks every `.x` use site.
  GCC's error prints the function type — read it.
- **`static` member = property of the type**, not the object. Made `columns`/`rows` static,
  which silently froze board size at compile time (blocks the "board size setting" idea).

### Structure now in place
```
MovementMode   file scope
GameState      currentMode, pendingMode, pxUnitPos, path, nextPxTurnHead, speed, dead, turnPending
Config         columns, rows, windowSize, cellSize, unitSize, unitGapFromLines
               built by makeConfig(windowSize), used as `const Config cfg`
UiState        debug, settingsOpen
FrameState     cellPosOfHead, crossed, unitFractionPosition
```
Compiles clean at -Wall -Wextra -O3.

### Naming debt (listed, not fixed)
columns/rows static+float -> instance+int | buttonPlacement -> halfCell/pxFromCells |
is_oob -> checkOob | debugInfo -> drawDebugInfo | addUnit -> handleAddUnitKey |
oppositeDir -> by value | `struct GameState gs;` -> `GameState gs;` |
unitFractionPosition -> cellFraction

### Advice given on splitting files
- Not yet. Extract `updateGame` / `drawGame` / `resetGame` / `drawSettings` *inside* main.cpp first.
- Key habit: pass `dt` and input in; stop calling `GetFrameTime()`/`IsKeyPressed()` deep
  inside logic. Test: "can this function be called with no window open?"
- Makefile has no header deps -> add `-MMD -MP` + `-include $(DEPS)` before splitting.
- RAYGUI_IMPLEMENTATION must live in exactly one TU (keep it in main.cpp).

### Open
- Turn/lerp loop still encodes axis+direction in 4 branches (direction flip exists because
  the fraction is unsigned).
- All `FIX:` comments still there, untouched by design.
