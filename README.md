# Naruto Senki V2 — Android Clean Base

This package is an Android-only working base extracted from the original Naruto Senki V2 source.

## Target

- Android only
- Cocos2d-x 2.2.6 native C++ engine
- Lua/LuaJIT gameplay/runtime support
- Android Studio + Gradle + NDK build
- `armeabi-v7a` and `arm64-v8a`

## Important directories

```text
cocos2dx/                         Cocos2d-x engine (Android platform retained)
CocosDenshion/                    Android audio engine
extensions/                       Cocos2d-x extensions
scripting/lua/                    Lua/LuaJIT runtime
external/sqlite3/                SQLite used by the project
projects/NarutoSenki/Classes/     Game C++ source
projects/NarutoSenki/Resources/  Game assets
projects/NarutoSenki/lua/       Game Lua scripts
projects/NarutoSenki/proj.android-studio/  Android build project
```

## Removed from this Android base

Windows, Linux, macOS, iOS game projects and their platform-specific engine implementations were removed. Development-only VS Code files, generic desktop build scripts, desktop tooling, and desktop-only GLFW assets were also removed.

The Android engine sources and Android third-party libraries required by `cocos2dx/Android.mk` were preserved.

## Build entry point

Open:

`projects/NarutoSenki/proj.android-studio/`

or run from that directory:

```bash
./gradlew assembleDebug
```

The project is intentionally kept close to the original Android build configuration. Do not upgrade Gradle/AGP/NDK all at once; first establish a reproducible baseline build, then modernize the toolchain in controlled steps.

## Custom APK workflow

1. Build the untouched Android baseline.
2. Change package/application identity and branding.
3. Modify C++ gameplay under `Classes/`.
4. Modify Lua gameplay/UI under `lua/`.
5. Modify assets under `Resources/`.
6. Add or replace characters, skills, maps, effects and UI.
7. Build a debug APK and test on device.
8. Configure a new release signing key for the custom version.
