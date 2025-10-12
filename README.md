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
| macOS    | \      | \      | Metal     | `-framework Cocoa -framework QuartzCore -framework Metal` |
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

**Linux**
```sh
gcc example/simple.c -ldl -lm -o simple && ./simple
```

## API Reference

### Window & Canvas Creation

#### canvas
```c
int canvas(int x, int y, int width, int height, const char *title)
```
Creates a canvas with full rendering backend setup. Returns the window ID.
- The platform's rendering backend is automatically initialized on first canvas creation
- A canvas is a window with graphics API ready to use

#### canvas_window
```c
int canvas_window(int x, int y, int width, int height, const char *title)
```
Creates a raw OS window without any rendering setup. Returns the window ID.
- Use this if you don't need the built-in rendering backend
- Lower-level alternative to `canvas()`

### Window Management

#### canvas_set
```c
int canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title)
```
Changes window position, size, display, and title.
- `display = -1` → primary display
- `x = -1` → center on screen x axis
- `y = -1` → center on screen y axis
- `title = NULL` → keep existing title

#### canvas_minimize
```c
int canvas_minimize(int window)
```
Minimizes the window.

#### canvas_maximize
```c
int canvas_maximize(int window)
```
Maximizes the window.

#### canvas_restore
```c
int canvas_restore(int window)
```
Restores the window from minimized or maximized state.

#### canvas_close
```c
int canvas_close(int window)
```
Closes the window and destroys any attached rendering backend data (swapchain, etc.).

#### canvas_cursor
```c
int canvas_cursor(int window_id, canvas_cursor_type cursor)
```
Sets the cursor type for a window.

Available cursor types:
- `ARROW`
- `TEXT`
- `CROSSHAIR`
- `HAND`
- `SIZE_NS` (north-south resize)
- `SIZE_EW` (east-west resize)
- `SIZE_NESW` (diagonal resize)
- `SIZE_NWSE` (diagonal resize)
- `SIZE_ALL` (move)
- `NOT_ALLOWED`
- `WAIT`

### Main Loop & Updates

#### canvas_run
```c
int canvas_run(canvas_update_callback update)
```
Starts the main loop, processing OS events, timing, and updates.

The callback signature is:
```c
typedef void (*canvas_update_callback)(int window);

void update(int window) {
    // Your per-window update code
}
```

#### canvas_set_update_callback
```c
int canvas_set_update_callback(int window, canvas_update_callback callback)
```
Sets a per-window override callback function. If not set, the default callback passed to `canvas_run()` is used.

#### canvas_quit
```c
int canvas_quit()
```
Signals the main loop to exit. Call this to cleanly shut down the application.

### Rendering

#### canvas_color
```c
int canvas_color(int window, const float color[4])
```
Sets the window's clear color (RGBA, values 0.0 to 1.0).

Example:
```c
canvas_color(window, (float[]){0.2f, 0.3f, 0.4f, 1.0f});
```

### Time Management

Canvas provides a robust high resolution timing system for frame timing, FPS calculation, and fixed timestep updates.

#### canvas_time_data structure
```c
typedef struct {
    uint64_t start;      // Start time reference
    double current;      // Current time in seconds since start
    double delta;        // Smoothed delta time in seconds
    double raw_delta;    // Unsmoothed delta time
    double fps;          // Current FPS (averaged over 60 frames)
    uint64_t frame;      // Frame counter
    double accumulator;  // For fixed timestep
    double alpha;        // Interpolation factor for fixed timestep
    double last;         // Last time value
    double times[60];    // For FPS smoothing
    int frame_index;     // Index for frame times
} canvas_time_data;
```

#### canvas_time_init
```c
void canvas_time_init(canvas_time_data *time)
```
Initializes a time data structure. Called automatically for the main loop and per-window timers.

#### canvas_time_update
```c
void canvas_time_update(canvas_time_data *time)
```
Updates timing information. Called automatically each frame by the main loop.

#### canvas_get_time
```c
double canvas_get_time(canvas_time_data *time)
```
Returns the current time in seconds since the timer was initialized.

#### canvas_time_fixed_step
```c
int canvas_time_fixed_step(canvas_time_data *time, double fixed_dt, int max_steps)
```
Implements fixed timestep logic for physics or game updates. Returns the number of fixed steps to execute.

Example usage:
```c
int steps = canvas_time_fixed_step(&time, 1.0/60.0, 10);
for (int i = 0; i < steps; i++) {
    // Fixed timestep update (e.g., physics)
}
// Use time.alpha for interpolation between states
```

#### canvas_limit_fps
```c
void canvas_limit_fps(canvas_time_data *time, double target_fps)
```
Limits the frame rate to the specified FPS. Uses sleep + busy-wait for accuracy.

#### canvas_sleep
```c
void canvas_sleep(double seconds)
```
Platform-agnostic sleep function.

### Global Time
The main loop maintains a global timer accessible via:
```c
extern canvas_time_data canvas_main_time;
```

Each window also has its own timer in `_canvas[window_id].time`.

## Configuration

Exposed settings you can define before including the header:

```c
// Maximum number of windows
#ifndef MAX_CANVAS
#define MAX_CANVAS 16
#endif

// Maximum number of displays
#ifndef MAX_DISPLAYS
#define MAX_DISPLAYS 8
#endif

// FPS limit for main loop (default: 240)
extern double canvas_limit_mainloop_fps;
```

## Window State

Each window tracks its state in the `canvas_type` structure:
```c
typedef struct {
    int index;           // Window ID
    int x, y;           // Position
    int width, height;  // Size
    int display;        // Display index
    bool resize;        // Needs resize
    bool close;         // Close requested
    bool titlebar;      // Has custom titlebar
    bool os_moved;      // Moved by OS
    bool os_resized;    // Resized by OS
    bool minimized;     // Is minimized
    bool maximized;     // Is maximized
    float clear[4];     // Clear color (RGBA)
    const char *title;  // Window title
    canvas_window_handle window;  // Native window handle
    canvas_update_callback update; // Per-window callback
    canvas_time_data time;         // Per-window timer
} canvas_type;
```

Access via:
```c
extern canvas_type _canvas[MAX_CANVAS];
```

## Display Information

Query display information:
```c
typedef struct {
    bool primary;
    int x, y;
    int width, height;
    int refresh_rate;
} canvas_display;

extern canvas_display _canvas_displays[MAX_DISPLAYS];
extern int _canvas_display_count;
extern int _canvas_highest_refresh_rate;
```

## Error Codes

```c
#define CANVAS_OK 0
#define CANVAS_FAIL -1
#define CANVAS_INVALID -2
#define CANVAS_ERR_LOAD_LIBRARY -10
#define CANVAS_ERR_LOAD_SYMBOL -11
#define CANVAS_ERR_NO_FREE -32
#define CANVAS_ERR_GET_DISPLAY -33
#define CANVAS_ERR_GET_WINDOW -34
#define CANVAS_ERR_GET_GPU -35
#define CANVAS_ERR_GET_PLATFORM -36
```

Check the global status:
```c
extern int CANVAS_STATUS;
```

## Logging

Define `CANVAS_NO_LOG` to disable all logging.

Define `CANVAS_LOG_DEBUG` to disable verbose/warning/error/debug logs but keep info logs.

## Example

```c
#include "canvas.h"

void update(int window) {
    // Set clear color to dark blue
    canvas_color(window, (float[]){0.1f, 0.2f, 0.3f, 1.0f});
    
    // Check if window should close
    if (_canvas[window].close) {
        canvas_quit();
    }
}

int main() {
    // Create a centered 800x600 window
    int window = canvas(-1, -1, 800, 600, "My Application");
    
    if (window < 0) {
        return -1;
    }
    
    // Run main loop with update callback
    canvas_run(update);
    
    // Cleanup
    canvas_close(window);
    canvas_exit();
    
    return 0;
}
```

## Support
Did you know this effort has gone 100% out of my pocket?
If you think this project speaks for itself, consider supporting on github sponsors to continue making
projects like these a reality, open & free.

Supporter or not, you can **always** reach me on <a href="https://discord.gg/cxRvzUyzG8">My Discord Server, my primary communication channel</a>
Questions, feedback or support related to any of my projects, or if you need consulting.

## License
Logos, Branding, Trademarks - Copyright Dawn Larsson 2022

Repository: Apache-2.0 license