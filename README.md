# Dawning Canvas API
Cross platform rendering backend & windowing with reasonable defaults.

Goals:
- Single file pure C
- Zero: Build system, dependencies, complex setup or config
- Multi windowed rendering
- Extremely lightweight & low overhead

> [!CAUTION]
> This repo is early & experimental

##  Platform Status
`~` not implemented, `x` - implemented, `\` partially implemented (or wip)
```sh
Platform:               Window  Canvas  Backend     Required Compiler Flags
Windows                 \       \       DirectX12   -lgdi32 -luser32 -mwindows -ldwmapi -ldxgi -ld3d12
MacOS                   \       \       Metal       -framework Cocoa
Linux                   \       ~       Vulkan      -lX11
iOS                     ~       ~       Metal
Android                 ~       ~       Vulkan
HTML5                   ~       ~       WebGPU
```
note for windows: `x86_64-w64-mingw32-gcc` for cross compiling to windows

mac build example:
```sh
clang example/simple.c -framework Cocoa -framework QuartzCore -framework Metal
```
# Canvas
```c
int canvas(int x, int y, int width, int height, const char *title)
```
Canvas is the full renderable surface.

the platforms rendering backend is automaticly setup the first time a canvas is created, then persists untill the end of the program.

```c
#include "canvas.h"

void update(int window)
{
        canvas_color(window, (float[]){window, 0.0f, 0.0f, 1.0f});
}

int main()
{
        int window_1 = canvas(400, 400, 600, 600, "My Window");
        int window_2 = canvas(500, 500, 600, 600, "My Window 2");

        return canvas_run(update);
}
```
### Builds out: ~35 kb on macos

note: use `_canvas_window` to create a empty window with the platform, this dosn't trigger the rendering backend to be setup.

# Calls

```c
int canvas_color(int window, const float color[4])
```
sets clear color, this is just a wrapper for the _canvas struct array
```c
_canvas[window].clear // = double clear[4];
```

# Info
Apache License 2.0, by Dawn Larsson
