# Findings

Known issues found during the C++23 / warnings-as-errors / global-state work that are
**not yet fixed**. Line numbers are as of commit `c26dc97`; they will drift as the code
changes, so prefer the surrounding code over the line number.

Each entry records what was actually observed versus what is reasoned. Nothing here has
been reproduced at runtime.

---

## 1. Out-of-bounds read of `pathpoint[-1]` when drawing pathfind links

**Severity:** medium (undefined behaviour, potential crash or garbage geometry)
**Status:** confirmed by reading code; not reproduced at runtime; no test coverage

`App/source/GameDraw.cpp:539` guards the pathfind-link drawing loop with only
`numpathpoints > 1`, then at `:554` indexes
`pathpoint[gamestate.pathpointselected]`.

`gamestate.pathpointselected` uses `-1` as a valid "nothing selected" sentinel. It is set
to `-1` at `App/source/GameTick.cpp:1691`, and `GameTick.cpp:1675` shows the correct
combined guard for exactly this case:

```cpp
if (numpathpoints > 1 && gamestate.pathpointselected != -1) {
```

So the codebase already knows the right idiom, and the delete path at
`GameTick.cpp:1705` is also correctly guarded. `GameDraw.cpp` is the one site missing the
`-1` check.

Reaching it requires more than one pathfind waypoint plus none selected, so it is narrow.
It is a read of one `Vector3` before the array.

**Suggested fix:** add `&& gamestate.pathpointselected != -1` to the `GameDraw.cpp:539`
guard, matching `GameTick.cpp:1675`.

---

## 2. Key-capture thread: unsynchronised handshake, and an exception path that skips the join

**Severity:** medium (use-after-free on the exception path; formally UB on the handshake)
**Status:** confirmed by reading code; not reproduced at runtime; no test coverage

The entire project has exactly **one** thread. There are no mutexes, atomics, or condition
variables anywhere in the tree:

```
App/source/Menu/Menu.cpp:1032   SDL_CreateThread(setKeySelected_thread, NULL, &gamestate)
```

It exists for one interaction: clicking a keybind in the options menu parks the main loop
and this thread blocks in `SDL_WaitEvent` until a key or mouse button arrives.

Three separate problems:

### 2a. The `waiting` flag is a handshake, not a synchronisation primitive

`setKeySelected_thread` writes `gamestate.keyselect` and `gamestate.waiting`
(`Menu.cpp:1008-1009`); the main thread polls them. They are plain non-atomic members, so
this is undefined behaviour by the standard even where it behaves correctly in practice on
x86/ARM. The flag write happens before the thread exits and `joinKeySelectThread` creates
a happens-before edge, which is why it works today - but nothing enforces the ordering
during the window the main thread is actually polling.

### 2b. The thread calls `Menu::Load`, which touches shared state

`Menu::Load(gamestate)` at `Menu.cpp:1010` writes `gamestate.mainmenu = 0` (`Menu.cpp:539`),
racing the main thread's `Menu::Tick`. It also mutates the function-local `static
Menu::items` vector (`MenuItem::effectfade`), which `Menu::handleFadeEffect` reads.

In practice the main thread is parked on `waiting` while the thread runs, so this does not
manifest - but that is a consequence of the handshake, not a design.

### 2c. An exception unwinds past the join, leaving a dangling reference

`Lugaru/source/main.cpp:616` calls `Menu::joinKeySelectThread()`, but the enclosing `catch
(const std::exception&)` at `main.cpp:626` is reached by an exception thrown while the
thread is still alive. The thread holds `GameState&` referring to the stack object
declared at `main.cpp:552`, which is destroyed as the stack unwinds. The thread is then
left holding a dangling reference and is never joined.

This is the same class of bug that was already fixed once in `Menu.cpp` (the thread was not
being joined at all); this is the remaining path.

**Suggested fix:** make `waiting` and `keyselect` `std::atomic`; guarantee the join on every
exit path (scope guard, or move the join so the `catch` cannot bypass it); and decide
whether `Menu::Load` needs to run on the thread at all, or whether the thread could set a
flag and let the main thread do the menu reload.

---

## 3. Editor: saved maps are written somewhere the loader never reads

**Severity:** medium (the editor's save/load loop does not work end to end)
**Status:** confirmed by reading code

The two operations use different roots:

- save writes to `Folders::getUserDataPath() + "/Maps"` - `App/source/Devtools/ConsoleCmds.cpp:195`
  (`save_json`) and `:274` (`save`, binary)
- load reads from `Folders::getResourcePath("Maps/" + name + ".json")` - `App/source/GameTick.cpp:857`,
  which resolves to `dataDir + "/Maps/..."` via `Foundation/include/Utils/Folders.hpp:68-71`

So `map foo` after `save_json foo` will not find the file; it has to be copied by hand from
`%APPDATA%\Lugaru\Maps\` into `Lugaru\Data\Maps\`.

This will be addressed as part of the planned editor work (a map picker covering both
locations), so it is recorded rather than fixed in isolation.

---

## 4. Editor: `editorsize` defaults to 0, so the first placed object has zero scale

**Severity:** low (surprising but harmless; nothing crashes)
**Status:** confirmed by reading code

`App/include/GameState.hpp:17` declares `float editorsize = 0;`. It is passed as the object
scale at `App/source/GameTick.cpp:1578` and `:1580`. Pressing `o` to place an object before
pressing `up` therefore places a zero-scale object.

There is no way to re-size or re-place an object after creation - `editorsize`,
`editoryaw` and `editorpitch` are all *next-object* state, so editing is strictly
create-or-delete.

The `0` default is faithful to the original global and was deliberately preserved during
migration. Changing it to something usable is a behaviour change and needs a decision.

---

## 5. Editor: two level-saving commands are undocumented

**Severity:** low (documentation gap)
**Status:** confirmed by reading code

`Docs/DEVTOOLS.txt` documents `map` and `save`, but not:

- `save_json` (`ConsoleCmds.cpp:193`) - writes JSON `version 13`, the format the loader
  prefers
- `convert_to_json` (`ConsoleCmds.cpp:266`) - loads then re-saves as JSON

Meanwhile the documented `save` (`ConsoleCmds.cpp:272`) writes the legacy **binary**
`mapvers 12`. So the documented command is the one you are least likely to want.

---

## 6. `Sprite::Draw` receives the same value twice

**Severity:** low (redundancy, no defect)
**Status:** confirmed by reading code

`App/include/Graphic/Sprite.hpp:83` takes `float gravity` as a positional parameter, while
the same call at `App/source/GameDraw.cpp:519` also passes `gamestate`. The body uses the
parameter in some places (`Sprite.cpp:297,341,421,424`) and the instance in others
(`Sprite.cpp:367`) - two sources of truth for one value in one call.

This matches existing project convention: `multiplier`, `bloodtoggle` and `windvector` are
passed the same way. Collapsing only `gravity` would be inconsistent, so the sensible fix
is to remove the redundant scalar parameters across `Sprite::Draw` / `Object::Draw` /
`Terrain::draw` as one coherent step, once `multiplier` is no longer a global.

---

## 7. Standing test-coverage gap

**Severity:** informational
**Status:** by design, recorded so it is not forgotten

All tests are unit or architecture tests. **None of them** loads a level, ticks a
simulation, constructs a `Person` in a running world, or renders a frame. Consequences:

- The globals migration was verified by source-level equivalence (mechanical line-pair and
  member-name diffing), not by observed runtime behaviour. A pre-existing bug that merely
  *moved* - finding 1 is exactly such a case - would not be caught.
- `GameState` defaults and injection behaviour are well covered, but actual gameplay
  behaviour is not.

Closing this needs headless level-loading infrastructure. Until then, gameplay changes need
manual playtesting, and findings like 1 should be treated as "never exercised" rather than
"known not to happen".

---

## 8. Process note: subagent self-reports need auditing

Not a code issue, but recorded because it affected how much a verification pass could be
trusted.

During the tranche 4 migration the implementing agent reported that `yaw` and `pitch` had
"22 and 6" use sites, "all in `App/source/GameTick.cpp`", and that `ConsoleCmds.def`
contains `yaw`/`pitch` macro names. The verification pass measured 20 and 17 across three
files, and `ConsoleCmds.def` contains neither - only `viewdistance` and `fadestart`.

The code was correct; the report was not. The same pattern appeared in the tranche 2 and 3
agents understating counts. Every claim is therefore verified independently rather than
trusted, and the audit trail in `AppTest/source/GlobalMigrationTest.cpp` exists partly
because self-reported numbers have been unreliable.