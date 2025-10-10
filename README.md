![Dawning Micro Header (1)](https://github.com/user-attachments/assets/e39801c7-7969-4de8-ae9f-236a492b57ac)

> [!CAUTION]
> This repo is early & experimental

Extremely easy to use, cross-platform rendering backend & windowing with reasonable defaults.
This aims to provide all the basics to kickstart engines, frameworks, or other tools or applications with the lowest possible barrier to entry without compromising on speed or bloat.

**Goals:**
- Single file pure C
- Zero: build system, dependencies, complex setup or config
- Multi-window rendering
- Extremely lightweight & low overhead

## Platform Status

`~` not implemented     `x` implemented     `\` partially implemented (WIP)

| Platform | Window | Canvas | Backend   | Required Compiler Flags |
|----------|--------|--------|-----------|------------------------|
| Windows  | \      | \      | DirectX12 | `-lgdi32 -luser32 -mwindows -ldwmapi -ldxgi -ld3d12` |
| macOS    | \      | \      | Metal     | `-framework Cocoa` |
| Linux    | \      | ~      | Vulkan    | `-ldl -lm` |
| iOS      | ~      | ~      | Metal     | |
| Android  | ~      | ~      | Vulkan    | |
| HTML5    | ~      | ~      | WebGPU    | |

**Note for Windows:** Use `x86_64-w64-mingw32-gcc` for cross-compiling to Windows

**Note for Linux:** Wayland is loaded first, falls back to x11

## Building the example

**Windows**
```sh
zig cc example/simple.c -I. -s -O3 -Qn -ldwmapi -ldxgi -ld3d12 && .\a.exe
```

**MacOS**
```sh
clang example/simple.c -framework Cocoa -framework QuartzCore -framework Metal
```

## Usage

### Canvas
```c
int canvas(int x, int y, int width, int height, const char *title)
```
A canvas is the full renderable surface. The platform’s rendering backend is automatically set up the first time a canvas is created, then persists until the end of the program.

### Window
```c
int canvas_window(int x, int y, int width, int height, const char *title)
```
Creates an empty raw OS window without any rendering setup or attached backend.

### Window Position, Width & Height
```c
int canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title);
```
Changes the window position and size, and optionally the display.
- `display = -1` -> primary display
- `x = -1` -> center on screen x axis
- `y = -1` -> center on screen y axis

### Window Maximize
```c
int canvas_minimize(int window);
```

### Window Minimize
```c
int canvas_maximize(int window);
```

### Window Restore
```c
int canvas_restore(int window);
```

### Window Close
```c
int canvas_close(int window);
```
Closes the window and destroys any attached rendering backend data (swapchain, etc.).

### Run
```c
int canvas_run(canvas_update_callback update);
```
Hands over control flow to canvas, processing OS events, timing, and updating.

`canvas_update_callback` - function pointer to your primary update function that runs per canvas or window:
```c
void update(int window) {
        // your code per window
}
```

### Set per canvas callback
```c
int canvas_set_update_callback(int window, canvas_update_callback callback);
```
Set's a override callback function for that window

### Sleep
```c
void canvas_sleep(double seconds)
```
Platform sleep

## Rendering Usage

### Color
```c
int canvas_color(int window, const float color[4]);
```
Sets the window’s clearing color rendered by the backend.

## Other usage
exposed settings:
```c
float canvas_limit_mainloop_fps = 240.0;

#ifndef MAX_CANVAS
#define MAX_CANVAS 16
#endif

#ifndef MAX_DISPLAYS
#define MAX_DISPLAYS 8
#endif
```


## Support
Did you know this effort has gone 100% out of my pocket?
If you think this project speaks for itself, consider supporting on github sponsors to continue making
projects like these a reality, open & free.

Supporter or not, you can **always** reach me on <a href="https://discord.gg/cxRvzUyzG8">My Discord Server, my primary communication channel</a>
Questions, feedback or support related to any of my projects, or if you need consulting.

## License
Logos, Branding, Trademarks - Copyright Dawn Larsson 2022

Repository:
Apache-2.0 license 

