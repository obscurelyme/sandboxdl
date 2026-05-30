# Tiny Breakout

This is a small game written in C++ using SDL3. It's my first original release game with SDL3.

Building for debug

```sh
cmake --preset=debug
cmake --build build/debug
```

Building for release

```sh
cmake --preset=release
cmake --build build/release
```

Package release

```sh
cpack --config build/release/CPackConfig.cmake
```

Windows executable icon

The Windows app icon is embedded at build time using a `.ico` file.

- Default icon path: `assets/app.ico`
- Override path with CMake cache variable: `APP_ICON_PATH`

```sh
cmake --preset=release -DAPP_ICON_PATH="C:/path/to/app.ico"
```

If the icon file does not exist, configure still succeeds and prints a status message.

## Credits:

Artwork:

- Artist: Buch
- Link: [Open Game Art](https://opengameart.org/users/buch)

---

Sounds:

- Artist: Kenny
- Link: [Open Game Art](https://opengameart.org/content/51-ui-sound-effects-buttons-switches-and-clicks)

---

- Artist: SubspaceAudio
- Link: [Open Game Art](https://opengameart.org/content/512-sound-effects-8-bit-style)
