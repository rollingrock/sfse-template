# sfse-template-plugin

A minimal "hello world" template for building a [Starfield Script Extender](https://www.nexusmods.com/starfield/mods/106) (SFSE) plugin.

It produces a single DLL that:

- Declares itself an SFSE plugin (`SFSEPlugin_Version` + `SFSEPlugin_Load`)
- Targets Starfield **1.16.236** / SFSE **0.2.19**
- Registers a messaging listener with SFSE and logs every lifecycle message it receives
- Writes a log to `Documents\My Games\Starfield\SFSE\Logs\sfse-template-plugin.log`

## Requirements

- Windows 10 / 11
- Visual Studio 2026 (with the "Desktop development with C++" workload) **or** MSVC build tools + Ninja
  - VS2022 also works — pass `-G "Visual Studio 17 2022"` to `cmake --preset default`, or use the `ninja` preset
- CMake 3.31 or newer (required for the VS2026 generator; the `ninja` preset works with 3.25+)
- (Optional, recommended) [vcpkg](https://github.com/microsoft/vcpkg) — used to pull in `spdlog`. Without it the template still builds and falls back to plain `std::ofstream` logging.

## Quickstart

```pwsh
# 1. Clone
git clone https://github.com/<you>/sfse-template-plugin.git
cd sfse-template-plugin

# 2. (optional) point CMake at your vcpkg install
$env:VCPKG_ROOT = "C:\vcpkg"

# 3. Configure and build
cmake --preset default
cmake --build --preset release
```

The plugin DLL ends up at `build/default/Release/sfse-template-plugin.dll`.

Drop that DLL into `<your Starfield install>\Data\SFSE\Plugins\` and launch the game via `sfse_loader.exe`.

## Using a local SFSE checkout

By default the CMake configure step uses `FetchContent` to clone `ianpatt/sfse` at tag `v0.2.19`. If you already have an SFSE source tree on disk, point CMake at it:

```pwsh
cmake --preset default -DSFSE_LOCAL_PATH="C:/path/to/sfse-0.2.19"
```

or set it as an environment variable:

```pwsh
$env:SFSE_LOCAL_PATH = "C:/path/to/sfse-0.2.19"
cmake --preset default
```

The path should be the directory containing `sfse/`, `sfse_common/`, and `xbyak/`.

## Auto-deploy after build

Set `STARFIELD_PATH` (env var or CMake cache var) to your Starfield install folder and the build will copy `sfse-template-plugin.dll` (and its `.pdb`) into `<STARFIELD_PATH>\Data\SFSE\Plugins\` after every successful build:

```pwsh
$env:STARFIELD_PATH = "C:\Program Files (x86)\Steam\steamapps\common\Starfield"
cmake --preset default
cmake --build --preset release
```

Leave it unset and the deploy step is a no-op.

## Opening in an IDE

- **Visual Studio 2026** — `File > Open > Folder...` and pick the repo root. VS reads `CMakePresets.json` and offers the `default` configuration. Build with `Build > Build All`. (VS2022 works too — open the folder and pick the `ninja` preset, or override the generator.)
- **VS Code** — install the CMake Tools extension, open the folder, pick the `default` preset when prompted.

Either flow gives you full IntelliSense and an F5-debuggable target.

## Renaming the plugin

Three places to change when you fork this:

1. `CMakeLists.txt` — the `project(...)` name (controls the DLL filename).
2. `vcpkg.json` — the `"name"` field.
3. `src/Plugin.cpp` — `kPluginName`, `kPluginAuthor`, and the `name`/`author` fields inside `SFSEPlugin_Version`.

## Targeting a different Starfield version

Edit the `compatibleVersions` array in `src/Plugin.cpp`. Available runtime macros are defined in `sfse_common/sfse_version.h` (e.g. `RUNTIME_VERSION_1_15_216`, `RUNTIME_VERSION_1_14_70`).

To pin a different SFSE release, change `SFSE_GIT_TAG` in `cmake/FetchSFSE.cmake` (or pass `-DSFSE_GIT_TAG=...` at configure time).

## Project layout

```
sfse-template-plugin/
├── CMakeLists.txt              top-level build
├── CMakePresets.json           presets (default = VS2022 x64, ninja also available)
├── vcpkg.json                  manifest (spdlog)
├── cmake/
│   ├── FetchSFSE.cmake         local SFSE override / FetchContent fallback
│   └── DeployPlugin.cmake      optional post-build copy into Starfield
├── src/
│   ├── PCH.h                   precompiled header
│   └── Plugin.cpp              SFSEPlugin_Version + SFSEPlugin_Load
└── .github/workflows/build.yml CI: configure + build on windows-latest
```

## License

MIT — see [LICENSE](LICENSE).

SFSE itself is the work of Ian Patterson, Stephen Abel, and Expired, distributed under its own terms. This template only consumes SFSE's public headers; it does not redistribute SFSE binaries.
