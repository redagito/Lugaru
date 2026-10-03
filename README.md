# Lugaru

![Banner](./Docs/banner3.jpg)

Fork of Lugaru with a modern Conan/CMake build system, unit tests, and many
bugfixes. The ongoing goal is to finish splitting the game into layers and to
remove the remaining globals.

## Requirements

![Banner](./Docs/banner4.jpg)

Development is done with:

| Tool | Version |
|---|---|
| Conan | 2.x (developed against 2.32) |
| CMake | 4.x (developed against 4.4) |
| Compiler | MSVC 14.51 (Visual Studio 2026 Community) |
| Build tool | MSBuild (shipped with Visual Studio) |
| C++ standard | C++23 |

CMake drives the `Visual Studio 18 2026` multi-config generator, so a single
configure produces both Debug and Release. `make` and Ninja are **not** required
and are not installed on the development machine.

Note on Conan and Visual Studio 2026: Conan's settings schema only knows MSVC
toolset versions up to `195`, which is what it reports for VS2026. That is a
limitation of the schema, not the actual compiler -- CMake still compiles with
the real VS2026 toolset. Leave `compiler.version=195` in your profile so Conan
keeps reusing the cached prebuilt dependencies.

The language standard is set in two places that must agree:

* `compiler.cppstd=23` in your Conan profile
* `CMAKE_CXX_STANDARD 23` in the top level `CMakeLists.txt`

## Building

![Banner](./Docs/banner1.jpg)

Install the dependencies once per configuration. Conan writes the generated
CMake files into `build/generators`, so run both commands before configuring:

```
conan install . --build=missing -s build_type=Debug
conan install . --build=missing -s build_type=Release
```

`conan install` also (re)generates `CMakeUserPresets.json`. That file is
gitignored, so a fresh clone has to run `conan install` before the first
`cmake --preset`.

Configure:

```
cmake --preset conan-default
```

Build a configuration:

```
cmake --build build --config Debug
cmake --build build --config Release
```

The game executable is written to `build/<Config>/Lugaru/Lugaru.exe`, next to a
copy of the game data.

## Tests

![Banner](./Docs/banner1.jpg)

Every library layer has a sibling test project that references **only** that
library plus Catch2:

| Test project | Tests |
|---|---|
| `FoundationTest` | vector maths, rotations, line/triangle intersection, frustum culling, binary pack/unpack and byte order, image buffer sizing, path helpers |
| `AudioTest` | sound table integrity, ambient sound pool bounds |
| `GraphicsTest` | animation table integrity, animation files loaded from disk, `PersonType` |
| `GameTest` | `Hotspot`, `Account` progression, campaign selection, save/load round-trip |
| `AppTest` | award/bonus/console command tables, `Person` record limits |

Run them all through CTest:

```
ctest --test-dir build -C Debug
ctest --test-dir build -C Release
```

Or run a single suite directly:

```
build\FoundationTest\Debug\FoundationTest.exe
build\FoundationTest\Debug\FoundationTest.exe "[frustum]"   # filter by tag
```

Tests that construct game objects resolve `DATA_DIR` relative to the working
directory, so CTest runs them from the same folder as the game data.

## Design

![Banner](./Docs/banner5.jpg)

The game follows a layered design. Each layer is one library, and may only
reference layers below it:

```
-----------------------------------------------
|                  Lugaru                     | Lugaru Game (executable)
-----------------------------------------------
|                   App                       | Application Layer
-----------------------------------------------
|                   Game                      | Game Logic Layer
-----------------------------------------------
|                  Graphics                   | Graphics Layer
-----------------------------------------------
|                   Audio                     | Audio Layer
-----------------------------------------------
|                 Foundation                   | Foundation Layer
-----------------------------------------------
```

Each library is followed by its test project, for example `Foundation` and
`FoundationTest`.

Consequences worth knowing when adding code:

* A library must link without help from the executable. Anything the `App` layer
  needs from `main.cpp` (window state, SDL event handling, command line parsing)
  lives in `App/WindowContext.*` and `App/CommandLine.*`.
* Anything the audio subsystem owns (the ambient sound pool, slow motion pitch)
  lives in `Audio/AudioState.hpp` rather than in the application globals.
* A module may not be duplicated across layers under the same name. Several
  directories (`Graphic/`, `Objects/`, ...) exist in both `App/include` and
  `Graphics/include`, and both are public include roots, so prefer fully
  qualified includes and be aware of which one you get.

### Platform detection

`Platform/Platform.hpp` is the single source of truth. It defines
`LUGARU_PLATFORM_WINDOWS`, `LUGARU_PLATFORM_MACOS`, `LUGARU_PLATFORM_UNIX` and
`LUGARU_PLATFORM_LINUX` from the compiler's own macros, and `#error`s on an
unrecognised target. Do not test `WIN32`, `PLATFORM_UNIX` or `PLATFORM_LINUX`
directly -- those were never defined by the build system, which silently
compiled out every non-Windows code path.

### Binary file format

`Utils/binio.h` reads and writes the game's binary records. `funpackf` throws
`TruncatedFileException` when a stream ends early instead of unpacking
uninitialised heap memory, and `tryfunpackf` is the non-throwing variant used for
optional trailing sections (see `Graphics/source/Animation/Animation.cpp`, where
roughly 60 shipped animation files predate the `weapontarget` block).

## License

![Banner](./Docs/banner2.jpg)

The source code is distributed under the GNU General Public License version 2
or (at your option) any later version (GPLv2+). See `Docs/COPYING.txt`.

The assets (campaigns, graphical and audio assets, etc.) in the `Data` folder
are distributed under the Creative Commons Attribution - Share Alike license,
some in version 3.0 Unported (CC-BY-SA 3.0) and others in version 4.0
International (CC-BY-SA 4.0) as described in `Docs/CONTENT-LICENSE.txt`.