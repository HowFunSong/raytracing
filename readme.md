# RayTracer

multithreaded ray tracing program with multiple predefined scenes and dynamic progress bars.
This project follows the "[Ray Tracing in One Week](https://raytracing.github.io/)" tutorial series by Peter Shirley. For more details on the theory and implementation, see:

## Requirements

- C++17 compiler (e.g. `g++`)
- [indicators](https://github.com/p-ranav/indicators) library
- ImageMagick (`magick` command) for converting PPM output to PNG
- On Windows: Enable ANSI terminal processing (handled by the code via `enableVT()`)

## Build & Run

_All commands assume you are in the project root directory._

### 1. Clean & compile

```bash
# Remove old build/output folders
rm -rf build/ out/

# Create build directory
mkdir -p build

# Compile the program
# - Outputs executable to build/main.exe (Windows) or build/main (Unix)
g++ -g -std=c++17 -static-libstdc++ -Iinclude main.cpp -o build/main.exe

```

### 2. Run a scene

The executable accepts a single optional argument `<scene_number>`. If omitted or invalid, it defaults to the final scene.

```bash
# Usage information
build/main.exe           # prints usage, then runs default final scene
build/main.exe 5         # runs scene #5 (quads)
build/main.exe 3         # runs scene #3 (earth)
build/main.exe 9         # runs scene #9 (final_scene with high settings)
```

By default, `stdout` (the rendered PPM image and log) is printed to the console. To redirect the image to a file and save logs:

```bash
# Redirect stdout to log file, stderr (progress bars) remains in terminal\
build/main.exe 5 1> out/log.txt

# Convert the PPM image to PNG
magick out/img.ppm out/raytracer.png
```

### 3. Scenes

| #   | Function              | Description                       |
| --- | --------------------- | --------------------------------- |
| 1   | `bouncing_spheres()`  | Moving spheres on checker floor   |
| 2   | `checkered_spheres()` | Two large checker spheres         |
| 3   | `earth()`             | Textured Earth globe              |
| 4   | `perlin_spheres()`    | Spheres with Perlin noise texture |
| 5   | `quads()`             | Colored quads in 3D space         |
| 6   | `simple_light()`      | Simple noise floor + light source |
| 7   | `cornell_box()`       | Cornell box with textured sphere  |
| 8   | `cornell_smoke()`     | Cornell box with constant medium  |
| 9   | `final_scene()`       | Complex scene with BVH, lighting  |

## Example

Render the "quads" scene with progress bars visible, save output:

```bash
# 1) Build
g++ -g -std=c++17 -Iinclude main.cpp -o build/main.exe

# 2) Run quads scene, log to file
build/main.exe 5 1> out/run5.log

# 3) Convert output image
a magick out/img.ppm out/scene5.png
```

## Notes

- On Windows, ANSI mode and cursor hiding/restoring are handled automatically by `enableVT()` and `restoreCursor()`.
