# Native 3D FFI Demo

This demo uses Vyb FFI to call libc `system()`, then builds and launches a local
GLFW/OpenGL program. It opens a native window and renders orbiting cubes over a
grid until the window is closed.

Run it from the repository root:

```bash
build/vyb demos/ffi_native_3d/main.vyb
```

Requirements:

- `cc`
- GLFW
- OpenGL
- an active local display

The generated binary is written to `/tmp/vyb_native_cube`.

## Direction

This demo proves Vyb can reach native graphics today through FFI, but it is not
the intended long-term shape. The next step is a real Vyb 3D API that exposes
windowing, render loops, meshes, cameras, transforms, input, and shaders
directly from Vyb instead of shelling out to build and run a separate OpenGL C
program.
