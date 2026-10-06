# Findings

Known issues found during the C++23 / warnings-as-errors / global-state work that are
**not yet fixed**, plus a couple that have since been fixed and are kept for the record
(those say so in their status line). Line numbers are as of commit `c26dc97`, corrected
against the tree at `c81ff20`; they will drift as the code changes, so prefer the
surrounding code over the line number.

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
**Status:** confirmed by reading code; not reproduced at runtime; no test coverage. The join
hole in 2c is now closed by an RAII guard; 2a and 2b are still open.

The entire project has exactly **one** thread. There are no mutexes, atomics, or condition
variables anywhere in the tree:

```
App/source/Menu/Menu.cpp:1055   keyselectthread = SDL_CreateThread(setKeySelected_thread, NULL, args)
```

It exists for one interaction: clicking a keybind in the options menu parks the main loop
and this thread blocks in `SDL_WaitEvent` until a key or mouse button arrives.

Three separate problems:

### 2a. The `waiting` flag is a handshake, not a synchronisation primitive

`setKeySelected_thread` writes `gamestate.keyselect` and `gamestate.waiting`
(`Menu.cpp:1028-1029`); the main thread polls them. They are plain non-atomic members, so
this is undefined behaviour by the standard even where it behaves correctly in practice on
x86/ARM. The flag write happens before the thread exits and `joinKeySelectThread` creates
a happens-before edge, which is why it works today - but nothing enforces the ordering
during the window the main thread is actually polling.

### 2b. The thread calls `Menu::Load`, which mutates the shared menu item list

`Menu::Load(gamestate, assets)` at `Menu.cpp:1030` is the last thing the thread does. It
clears and rebuilds the file-static `Menu::items` vector (`Menu.cpp:58`), which
`Menu::handleFadeEffect` (`Menu.cpp:169-184`) walks and mutates on the main thread, and for
`mainmenu == 5` it calls `LoadCampaign` (`Menu.cpp:421`), which rewrites the global
`campaignlevels` and `campaignEndText`.

An earlier version of this finding claimed `Menu::Load` writes `gamestate.mainmenu = 0`.
That is wrong: `Menu::Load` spans `Menu.cpp:375-521` and contains no write to any
`GameState` member. The line it was citing, `Menu.cpp:541`, is inside
`Menu::startChallengeLevel` (`Menu.cpp:523`), a different function that the thread never
calls. What races is the item vector, not `mainmenu`.

In practice the main thread is parked on `waiting` while the thread runs, so this does not
manifest - but that is a consequence of the handshake, not a design.

### 2c. An exception unwinds past the join, leaving a dangling reference

The thread holds a `GameState&` and a `GameAssets&`, so it must be joined before either
object is destroyed. `Lugaru/source/main.cpp:630` does that explicitly, but the enclosing
`catch (const std::exception&)` at `main.cpp:640` is reached by an exception thrown while
the thread is still alive, and that path never passes the join at `:630`.

`GameState gamestate` (`main.cpp:554`) and `GameAssets assets` (`main.cpp:560`) are stack
objects, so on unwind `assets` is destroyed first and `gamestate` second. The thread's
references are therefore dangling from the moment `assets` begins to destruct, for the
whole of its destructor. That window is long: `~GameAssets` destroys the two fonts, each a
`glDeleteLists` (`Graphics/source/Graphic/Text.cpp:140`), and then the skybox and its 18
`Texture` members (8 singles plus `Mainmenuitems[10]`), each of which can drop the last
reference to a `TextureRes` and so run a `glDeleteTextures`
(`Graphics/source/Graphic/Texture.cpp:106-110`). Before commit `d70a1a9` the textures were
still globals, so the only work in that destructor was the two fonts.

This is the same class of bug that was already fixed once in `Menu.cpp` (the thread was not
being joined at all); this is the remaining path.

**Suggested fix:** make `waiting` and `keyselect` `std::atomic`; and decide whether
`Menu::Load` needs to run on the thread at all, or whether the thread could set a flag and
let the main thread do the menu reload. The join guarantee is no longer on this list: it is
now an RAII guard, `JoinKeySelectThreadOnExit` (`main.cpp:486`), declared after both objects
at `main.cpp:567` and therefore destroyed before them on every exit path out of the block -
fall-through, early return, or unwind. The explicit join at `:630` stays because
`deleteGame` deletes GL objects the thread would otherwise still be reading.

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
**Status:** fixed - the default is now `1`

`App/include/GameState.hpp:17` declared `float editorsize = 0;`. It is passed as the object
scale at `App/source/GameTick.cpp:1578` and `:1580`. Pressing `o` to place an object before
pressing `up` therefore placed a zero-scale object.

There is no way to re-size or re-place an object after creation - `editorsize`,
`editoryaw` and `editorpitch` are all *next-object* state, so editing is strictly
create-or-delete.

The `0` default was faithful to the original global and was deliberately preserved during
migration, but keeping it is a behaviour change in the wrong direction, so it has been
changed to `1`. Pinned by `56b253d` ("Assert a fresh editor size is usable"). Derived from
the code rather than picked: the increment is `gamestate.editorsize += gamestate.multiplier`
once per tick while `up` is held, and `multiplier` is the frame delta clamped to
`[.001, .6]` (`Lugaru/source/main.cpp:282-288` and `:317-319`), so one second of holding
adds about `1.0` and the decrement floors at `.1`. One second of holding `up` therefore lands
on `1`, which is also the nominal object scale - `Object::Object` calls
`model.Scale(.3 * scale, .3 * scale, .3 * scale)` (`Object.cpp:161`) - and the value
`Object::LoadObjects` falls back to when a scale cannot be read (`Object.cpp:601`). `1` also
puts `scale` on the `> .5` side of the branch at `Object.cpp:114`, so a placed rock gets the
friction `1.5` rather than `.5`.

---

## 5. Editor: four console commands are undocumented

**Severity:** low (documentation gap)
**Status:** fixed - all four are now documented

`Docs/DEVTOOLS.txt` documents `map` and `save`, but not:

- `save_json` (`ConsoleCmds.cpp:193`) - writes JSON `version 13`, the format the loader
  prefers
- `convert_to_json` (`ConsoleCmds.cpp:266`) - loads then re-saves as JSON
- `belt` (`ConsoleCmds.cpp:536`) - toggles `skeleton.clothes`, which gates the draw of the
  clothing mesh, so it hides and shows the clothes without removing them
- `default` (`ConsoleCmds.cpp:708`) - resets the main player's armor, protection, metal,
  health, speed, scale and proportions, clears their clothes, cancels `immobile`, and sets
  `gamestate.editoractive` back to `typeactive`

Meanwhile the documented `save` (`ConsoleCmds.cpp:272`) writes the legacy **binary**
`mapvers 12`. So the documented command is the one you are least likely to want.

The sweep that found these was written first, as `every console command is documented` in
`AppTest/source/ConsoleTableTest.cpp`: it reuses the `DECLARE_COMMAND` extraction the table
tests already do, so the list cannot drift from the code, and it guards on the extracted
count (62) so a parser that matched nothing cannot pass. It reported **four** missing
commands rather than the two originally expected here; `belt` and `default` are real
commands with no entry in the file, so all four were documented.

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

## 8. `Terrain::terraintexture` was a dead member that shadowed a migrated name

**Severity:** low (no runtime effect; a trap for the next reader)
**Status:** fixed - the member has been deleted

`Terrain` carried a `Texture terraintexture;` member that nothing read or wrote. Verified
before removal: no `terrain.terraintexture` and no `->terraintexture` anywhere in the tree,
and the only remaining `terraintexture` uses after commit `d70a1a9` are the
`assets.terraintexture` calls in `App/source/GameTick.cpp` and `App/source/GameDraw.cpp`,
which go through `GameAssets`.

It was already dead before the textures moved, but the migration turned it into a hazard.
Once `terraintexture` was a member of `GameAssets`, this declaration was the only bare
occurrence of the name in production code, so `rg terraintexture` lands on it first and a
reviewer can easily attribute the `GameAssets` wiring to `Terrain`, or reintroduce the
member. It also cost every `Terrain` instance a `shared_ptr`.

---

## 9. Process note: subagent self-reports need auditing

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

---

## 10. SHELVED: the eyeball rendering is missing its second half

**Severity:** unknown - looks like an unfinished feature, not a defect
**Status:** shelved deliberately on 2026-10-06; do not "clean up"

`Assets::cornea` and `Assets::iris` are loaded in `Game::LoadStuff`
(`App/source/GameInitDispose.cpp`) but nothing ever draws them. Grepping either name
finds no reader anywhere outside that load block, so the game pays the mesh load and the
heap for two models that never reach a frame.

This is recorded as a **suspected bug rather than dead code**, on the judgement that the
eyeball rendering was meant to draw all three parts and does not. `eye` itself is loaded
the same way and scaled to `.03`, which is the scale a model drawn at the camera's near
plane would use - consistent with an eye/cornea/iris trio that was drawn together once
and lost the caller.

**Deliberately left in place.** The obvious-looking cleanup here is to delete the two loads
and the two `GameAssets` members, which would be wrong: it would destroy the evidence and
make restoring the feature mean re-adding the assets, the loads and the scale calls. A
comment at the load site records the same thing for anyone reading the code rather than
this document.

To restore: draw `eye`, `cornea` and `iris` together in `Game::DrawGLScene` at the
camera's near plane, at `.03` scale, then delete the loads and the comment together with
the feature.