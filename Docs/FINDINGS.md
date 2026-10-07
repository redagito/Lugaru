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
**Status:** fixed - the draw guard now also tests for the `-1` sentinel

`App/source/GameDraw.cpp:539` guarded the pathfind-link drawing loop with only
`numpathpoints > 1`, then at `:554` indexed
`pathpoint[gamestate.pathpointselected]`.

`gamestate.pathpointselected` uses `-1` as a valid "nothing selected" sentinel. It is set
to `-1` at `App/source/GameTick.cpp:1691`, and `GameTick.cpp:1675` shows the correct
combined guard for exactly this case:

```cpp
if (numpathpoints > 1 && gamestate.pathpointselected != -1) {
```

So the codebase already knows the right idiom, and the delete path at
`GameTick.cpp:1705` is also correctly guarded. `GameDraw.cpp` was the one site missing the
`-1` check.

Reaching it requires more than one pathfind waypoint plus none selected, so it is narrow.
It is a read of one `Vector3` before the array.

The suggested fix was applied: the guard around the pathfind-link block now reads
`if (gamestate.numpathpoints > 1 && gamestate.pathpointselected != -1) {`, which is the
`GameTick.cpp` idiom above with the member qualification the block already used. Nothing
else moved - the same links and the same marker point are drawn, from the same coordinates,
for the same selections; only the state where nothing is selected stops drawing anything at
all, which is what the block was already doing for zero or one waypoint. Pinned by
`532495f` ("Assert the path-link draw guards against no selection"), as `no file indexes the path
point graph with an unselected point` in `AppTest/source/PathPointGuardTest.cpp`. That assertion
walks every `.c`, `.cpp`, `.h` and `.hpp` under `App/include` and `App/source`, and requires each
subscript made with the selection to sit inside a block whose condition tests for `-1`.

**Known gap, now fixed:** `GameTick.cpp:1659-1667` had the same defect. The connect
command's first half ran inside a `numpathpoints > 1` guard with no `-1` test, and
`i != gamestate.pathpointselected` is trivially true when the selection is `-1`, so with
nothing selected and the player near a path point it incremented
`numpathpointconnect[-1]` and wrote `pathpointconnect[-1][n]`. Given the member order at
`App/include/GameState.hpp:41-44`, `numpathpointconnect[-1]` overlaps the tail of
`pathpoint[29]` and `pathpointconnect[-1]` overlaps the tail of `numpathpointconnect`, so
that one was a write into live graph state rather than merely a read before the array.

The fix is the one line this gap was left waiting for, applied by `a24c617` ("Skip connecting
path points when nothing is selected"):

```cpp
if (gamestate.numpathpoints > 1 && gamestate.pathpointselected != -1) {
```

which is the `GameTick.cpp:1676` idiom further down the same keybind, with the member
qualification the block already used. Nothing else in the block moved - same loop bounds, same
coordinates, same `alreadyconnected` logic, same insertion. `i != gamestate.pathpointselected` at
`:1657` stays: with a real selection it is the only thing stopping the command from connecting
a point to itself when the player is standing on the selected one, so it is not redundant with
the new guard, only newly necessary to reason about separately.

One consequence worth naming: with nothing selected and the player near an existing point, the
old code set `connected` before corrupting the table, so the `if (!connected)` at `:1672` never
ran and no point was added. Now that the block is skipped, that key adds a point and selects it,
which is what the same key already did whenever there were zero or one points. That is the
guard's doing, not an extra change.

The sweep that found it is now tree-wide rather than scoped to `GameDraw.cpp`, and it holds
over all 15 subscripts the two files make with the selection - the marker point's x, y and z,
and the twelve in `GameTick.cpp` - pinned by `df0ebd9` ("Extend the path-point guard check to the
whole tree") as `no file indexes the path point graph with an unselected point` in
`AppTest/source/PathPointGuardTest.cpp`. No other unguarded site turned up.

---

## 2. Key-capture thread: unsynchronised handshake, and an exception path that skips the join

**Severity:** medium (use-after-free on the exception path; formally UB on the handshake)
**Status:** closed. The join hole in 2c is closed by an RAII guard; 2a and 2b are closed by
giving the handshake an owner of its own (`KeyCapture`) and moving the menu rebuild onto the
main thread. Pinned by `AppTest/source/KeyCaptureHandshakeTest.cpp`. Nothing here was ever
reproduced at runtime, so the closing argument is the source plus the tests rather than a run
that used to fail.

The entire project has exactly **one** thread:

```
App/source/Menu/Menu.cpp:1083   keyselectthread = SDL_CreateThread(setKeySelected_thread, NULL, args)
```

It exists for one interaction: clicking a keybind in the options menu parks the main loop
and this thread blocks in `SDL_WaitEvent` until a key or mouse button arrives.

Three separate problems were recorded. All three are closed now:

### 2a. CLOSED: the `waiting` flag was a handshake, not a synchronisation primitive

`setKeySelected_thread` wrote `gamestate.keyselect` and `gamestate.waiting`; the main thread
polled them. They were plain non-atomic members, so that was undefined behaviour by the
standard even where it behaves correctly in practice on x86/ARM. The flag write happened
before the thread exited and `joinKeySelectThread` created a happens-before edge, which is why
it worked - but nothing enforced the ordering during the window the main thread was
actually polling.

They cannot be atomic where they were. `std::atomic` is not copyable, and a member of that
type would cost `GameState` the trivial copyability and trivial destructibility
`GameStateTest.cpp:109-110` asserts, which is what lets it be built and destroyed in a unit
test with no GL context. So the handshake moved to a struct of its own, `KeyCapture`
(`App/include/KeyCapture.hpp`), holding

```
std::atomic<bool> waiting;
std::atomic<int> keyselect;
std::atomic<bool> reloadRequested;
```

It is default constructed once in `main()` (`main.cpp:566`) beside `GameState` and
`GameAssets`, and passed by reference down the chain those two already travel: `SetUp`,
`Game::InitGame`, `Game::Tick`, `Game::ProcessInput`, `Game::DrawGLScene`,
`Game::inputText`, `Menu::Load`, `Menu::Tick`, `Menu::updateControlsMenu`,
`Menu::setKeySelected`. There is deliberately no accessor, no global and no file-local
static - a shared owner behind a getter is the global it replaces with the storage hidden,
and that shape was already tried in this tree once and reverted.

`waiting` carries two meanings and carried them before the move as well: the capture
thread's flag, and `Game::inputText`'s "SDL text input has been started and not yet
submitted". They are mutually exclusive in practice, so they stayed one flag.

Ordering, since it is the reason the members are atomic rather than just concurrent: the
thread writes the captured scancode into `gamestate`, then `keyselect = -1`, then
`reloadRequested = true`, then `waiting = false`. `Menu::setKeySelected` stores
`waiting = true` on the main thread *before* calling `SDL_CreateThread`, so that store is
earlier than the thread's in `waiting`'s modification order, and the first load the main
thread makes of `waiting` after the thread starts can therefore only read the thread's
store. All three members are `std::atomic` with the default `memory_order_seq_cst`, so that
store is a release and that load is an acquire, and every write the thread made before it -
the captured keybind included - happens before everything the main thread does after it
observes the flag down. `takeReloadRequest()`'s `reloadRequested.exchange(false)` is a
second acquire on the same variable, which is what makes the keybind visible to the
`Menu::Load` that 2b moved onto the main thread.

### 2b. CLOSED: the thread used to call `Menu::Load`, which mutates the shared menu item list

`Menu::Load(gamestate, assets)` was the last thing the thread did. It clears and rebuilds
the file-static `Menu::items` vector (`Menu.cpp:59`), which `Menu::handleFadeEffect`
(`Menu.cpp:170-185`) walks and mutates on the main thread, and for `mainmenu == 5` it calls
`LoadCampaign` (`Menu.cpp:422`), which rewrites the global `campaignlevels` and
`campaignEndText`.

An earlier version of this finding claimed `Menu::Load` writes `gamestate.mainmenu = 0`.
That is wrong, and still is: `Menu::Load` spans `Menu.cpp:376-522` and contains no write to
any `GameState` member. The line it was citing, `Menu.cpp:542`, is inside
`Menu::startChallengeLevel` (`Menu.cpp:524`), a different function that the thread never
calls. What raced is the item vector, not `mainmenu`.

In practice the main thread was parked on `waiting` while the thread ran, so this did not
manifest - but that was a consequence of the handshake, not a design.

The thread now records the key it captured, sets `reloadRequested`, and returns.
`Menu::Tick` takes the request and calls `Menu::Load(gamestate, assets, keycapture)` itself,
on the main thread, exactly where the thread used to. It is the first statement of
`Menu::Tick` (`Menu.cpp:558`), outside every branch in it, because the controls-menu input
handling below it only runs once `waiting` is already clear - a consume placed after that
would never run on the frame the request arrives on. `takeReloadRequest()` consumes the
request with `exchange(false)`, so one rebind produces one reload rather than one per frame.

Closed alongside it, and only because of the same move: the thread also reached
`assets.Mainmenuitems` and `assets.Mapcircletexture` through the `Menu::Load` it called, on
the way to building the menu for `mainmenu` 1, 2 or 5. That was unreachable only because the
thread happened to arrive at `mainmenu` 3 or 4 - nothing enforced that, and the only thing
holding it back was the same handshake as above. With the call gone the thread holds no
`GameAssets&` at all: `KeySelectArgs` carries a `GameState*` and a `KeyCapture*` and nothing
else, so the route is closed by the shape of the argument record rather than by a
coincidence. `fireSound()` still reaches the audio library's `samp` and `channels` tables by
non-atomic read, which is unchanged and was already the case; it is a read of tables the main
thread only reallocates when a level loads, and the capture thread cannot outlive a load.

### 2c. An exception unwinds past the join, leaving a dangling reference

The thread holds a `GameState&` and a `KeyCapture&`, so it must be joined before either
object is destroyed. `Lugaru/source/main.cpp:636` does that explicitly, but the enclosing
`catch (const std::exception&)` at `main.cpp:646` is reached by an exception thrown while
the thread is still alive, and that path never passes the join at `:636`.

`GameState gamestate` (`main.cpp:555`), `GameAssets assets` (`main.cpp:561`) and
`KeyCapture keycapture` (`main.cpp:566`) are stack objects, so on unwind they are destroyed
in reverse declaration order and `gamestate` last, while the thread's references are dangling
from the moment the first of them begins to destruct. How long that window was depended on
what the thread referenced. It used to hold a `GameAssets&`, so the window was
`~GameAssets`: the two fonts, each a `glDeleteLists`
(`Graphics/source/Graphic/Text.cpp:140`), and then the skybox and its 18 `Texture` members
(8 singles plus `Mainmenuitems[10]`), each of which can drop the last reference to a
`TextureRes` and so run a `glDeleteTextures`
(`Graphics/source/Graphic/Texture.cpp:106-110`). It holds a `KeyCapture&` now, whose
destructor is trivial - but `~GameAssets` is still the longest one in the block, and
`deleteGame` is still called before it, so the window did not go away; it moved.

This is the same class of bug that was already fixed once in `Menu.cpp` (the thread was not
being joined at all); this is the remaining path.

**Suggested fix:** done; see 2a and 2b. The join guarantee is not on this list: it is an RAII
guard, `JoinKeySelectThreadOnExit` (`main.cpp:487`), declared after all three objects it
protects at `main.cpp:573` and therefore destroyed before them on every exit path out of the
block - fall-through, early return, or unwind. The explicit join at `:636` stays because
`deleteGame` tears down state the thread still references, and it has to happen before
`~GameAssets` runs.

---

## 3. Editor: saved maps are written somewhere the loader never reads

**Severity:** medium (the editor's save/load loop does not work end to end)
**Status:** fixed - the loader searches both Maps folders

The two operations used different roots:

- save writes to `Folders::getUserDataPath() + "/Maps"` - `App/source/Devtools/ConsoleCmds.cpp:195`
  (`save_json`) and `:274` (`save`, binary)
- load read only from `Folders::getResourcePath("Maps/" + name + ".json")` - `App/source/GameTick.cpp:857`,
  which resolves to `dataDir + "/Maps/..."` via `Foundation/include/Utils/Folders.hpp:68-71`

So `map foo` after `save_json foo` did not find the file; it had to be copied by hand from
`%APPDATA%\Lugaru\Maps\` into `Lugaru\Data\Maps\`.

Saving is unchanged - the user data directory is the right place for user-created content,
and moving it would break existing workflows. `Folders::findMapPath(name, extension)` in
`Foundation/include/Utils/Folders.hpp` resolves a map name to a path, and the loader calls it
from both `Game::LoadJsonLevel` and the binary branch of `Game::LoadLevel`. It searches the
user data `Maps` folder first and falls back to the resource `Maps` folder, returning an empty
string when the map is in neither, so a missing map is still reported as missing instead of
being opened from a path that does not exist.

The consequence of that ordering is that a user map now shadows a shipped map of the same
name: a `Data/Maps/tutorial.json` that has been edited and saved to the user data folder is
the one that loads. That is the point of the change - a map the user just saved is the one
they mean - but it does mean an edited copy cannot be un-shadowed without deleting or renaming
it, and it is the reason the map picker should show both locations rather than merging them.

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