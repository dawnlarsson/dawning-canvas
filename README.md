<img width="1280" height="225" alt="Slide 16_9 - 25 (1)-min" src="https://github.com/user-attachments/assets/415053c3-17b1-4de2-8083-b78bdc06adcc" />

> [!CAUTION]
> This repo is early, consider this in alpha

Extremely easy to use, cross-platform rendering backend & windowing with reasonable defaults.

This aims to provide all the basics to kickstart engines, frameworks, or other tools or applications with the lowest possible barrier to entry without compromising on speed or bloat.

**Features:**
- **Single file library** - Drop `canvas.h` into your project and go. No config, no setup. no bs.
- **Zero dependencies** - No 3rd party libraries, direct OS API calls
- **Rendering backends** - D3D12, Metal, Vulkan automatically selected
- **Multi-window support** - Create and manage multiple windows simultaneously
- **Mouse/Touch input** - Multi-monitor Mouse, touch, velocity tracking and history
- **High-resolution timing** - Frame timing, FPS calculation, fixed timestep support
- **Display management** - Multi-monitor data & tracking
- **Unified GPU Buffers**

**Roadmap:**
- Keyboard Input
- Unified rendering calls
- Audio?
- Bug squash, perf optimizations

## Platform Status

`~` not implemented     `x` implemented     `\` partially implemented (WIP)

| Platform | Window | Canvas | Backend   | Required Compiler Flags |
|----------|--------|--------|-----------|------------------------|
| Windows  | \      | \      | DirectX12 | `-lgdi32 -luser32 -mwindows -ldwmapi -ldxgi -ld3d12 -lwinmm` |
| macOS    | \      | \      | Metal     | `-framework Cocoa -framework QuartzCore -framework Metal -framework IOKit` |
| Linux    | \      | ~      | Vulkan    | `-ldl -lm` |
| iOS      | ~      | ~      | Metal     | |
| Android  | ~      | ~      | Vulkan    | |
| HTML5    | ~      | ~      | WebGPU    | |

**Note for Windows:** Use `x86_64-w64-mingw32-gcc` for cross-compiling to Windows

**Note for Linux:** Wayland is loaded first, falls back to x11

## Validation

`-DCANVAS_VALIDATION`=N
```
 *   0 = Off (default)
 *   1 = Errors only
 *   2 = Errors + warnings
 *   3 = Errors + warnings + info
 *   4 = Errors + warnings + info + verbose
 *   5 = All messages including debug/trace
```

## Building the example

**Windows**
```sh
zig cc example/simple.c -I. -s -O3 -Qn -ldwmapi -ldxgi -ld3d12 && .\a.exe
```

**MacOS**
```sh
clang example/simple.c -framework Cocoa -framework QuartzCore -framework Metal -framework IOKit -I./
```

**Linux**
```sh
gcc example/simple.c -ldl -lm -o simple -I./ && ./simple
```

## API Reference

### Window & Canvas Creation

#### canvas
```c
int canvas(int64_t x, int64_t y, int64_t width, int64_t height, const char *title)
```
Creates a canvas with full rendering backend setup. Returns the window ID.
- The platform's rendering backend is automatically initialized on first canvas creation
- A canvas is a window with graphics API ready to use

#### canvas_window
```c
int canvas_window(int64_t x, int64_t y, int64_t width, int64_t height, const char *title)
```
Creates a raw OS window without any rendering setup. Returns the window ID.
- Use this if you don't need the built-in rendering backend
- Lower-level alternative to `canvas()`

### Window Management

#### canvas_set
```c
int canvas_set(int window_id, int display, int64_t x, int64_t y, int64_t width, int64_t height, const char *title)
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

### Cursor & Pointer System

#### canvas_cursor
```c
int canvas_cursor(int window_id, canvas_cursor_type cursor)
```
Sets the cursor type for a window.

**Available cursor types:**
- `CANVAS_CURSOR_HIDDEN` - Hide the cursor completely
- `CANVAS_CURSOR_ARROW` - Standard arrow pointer
- `CANVAS_CURSOR_TEXT` - I-beam text cursor
- `CANVAS_CURSOR_CROSSHAIR` - Crosshair cursor
- `CANVAS_CURSOR_HAND` - Pointing hand
- `CANVAS_CURSOR_SIZE_NS` - North-south resize arrows (↕)
- `CANVAS_CURSOR_SIZE_EW` - East-west resize arrows (↔)
- `CANVAS_CURSOR_SIZE_NESW` - Diagonal resize (↗↙)
- `CANVAS_CURSOR_SIZE_NWSE` - Diagonal resize (↖↘)
- `CANVAS_CURSOR_SIZE_ALL` - Move/drag cursor
- `CANVAS_CURSOR_NOT_ALLOWED` - Not allowed/prohibited
- `CANVAS_CURSOR_WAIT` - Wait/busy cursor

**Example:**
```c
canvas_cursor(window, CANVAS_CURSOR_HAND);
```

#### Pointer Tracking

Canvas provides a sophisticated pointer tracking system for mouse, touch, and pen input with velocity calculation and history.

**Get a pointer:**
```c
canvas_pointer *canvas_get_pointer(int id); // 0 = primary mouse
```

**Pointer structure:**
```c
typedef struct {
    // Identity
    int id;                   // Unique ID (0 for mouse, touch ID for fingers)
    canvas_pointer_type type; // MOUSE, TOUCH, or PEN
    int window_id;            // Which window owns this pointer
    
    // Position
    int64_t x, y;                 // Window-relative coordinates
    int64_t screen_x, screen_y;   // Screen-relative coordinates
    int display;              // Display index
    
    // State
    uint32_t buttons;          // Bitmask of currently pressed buttons
    uint32_t buttons_pressed;  // Buttons pressed this frame
    uint32_t buttons_released; // Buttons released this frame
    
    float scroll_x, scroll_y;  // Scroll wheel delta (this frame)
    float pressure;            // 0.0-1.0 (for pen/touch)
    
    bool active;               // Is pointer currently tracked?
    bool inside_window;        // Is pointer inside window bounds?
    bool captured;             // Is pointer captured (for drag ops)?
    bool relative_mode;        // FPS-style relative motion
    
    canvas_cursor_type cursor; // Current cursor type
} canvas_pointer;
```

**Button state queries:**
```c
bool canvas_pointer_down(canvas_pointer *p, canvas_pointer_button btn);
bool canvas_pointer_pressed(canvas_pointer *p, canvas_pointer_button btn);
bool canvas_pointer_released(canvas_pointer *p, canvas_pointer_button btn);
```

**Button flags:**
- `CANVAS_BUTTON_LEFT`
- `CANVAS_BUTTON_RIGHT`
- `CANVAS_BUTTON_MIDDLE`
- `CANVAS_BUTTON_X1`
- `CANVAS_BUTTON_X2`

**Motion data:**
```c
float canvas_pointer_velocity(canvas_pointer *p);              // pixels/sec
float canvas_pointer_direction(canvas_pointer *p);             // radians
void canvas_pointer_delta(canvas_pointer *p, int *dx, int *dy); // Movement since last frame
```

**Example usage:**
```c
void update(int window) {
    canvas_pointer *mouse = canvas_get_pointer(0);
    
    if (mouse && mouse->inside_window) {
        // Check button state
        if (canvas_pointer_pressed(mouse, CANVAS_BUTTON_LEFT)) {
            printf("Clicked at (%d, %d)\n", mouse->x, mouse->y);
        }
        
        // Get motion data
        float velocity = canvas_pointer_velocity(mouse);
        int dx, dy;
        canvas_pointer_delta(mouse, &dx, &dy);
        
        // Handle scroll
        if (mouse->scroll_y != 0) {
            zoom += mouse->scroll_y * 0.1f;
        }
    }
}
```

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

#### canvas_exit
```c
int canvas_exit()
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

#### GPU Buffers

```c
typedef enum
{
    CANVAS_BUFFER_VERTEX,
    CANVAS_BUFFER_INDEX,
    CANVAS_BUFFER_UNIFORM,
    CANVAS_BUFFER_STORAGE,
} canvas_buffer_type;

typedef enum
{
    CANVAS_BUFFER_STATIC,  // Write once, read many
    CANVAS_BUFFER_DYNAMIC, // Write every frame, persistently mapped
    CANVAS_BUFFER_STAGING, // CPU->GPU transfer
} canvas_buffer_usage;

typedef struct
{
    void *platform_handle; // ID3D12Resource*, VkBuffer, or MTLBuffer
    void *mapped;          // Persistently mapped pointer (dynamic only)
    size_t size;
    canvas_buffer_type type;
    canvas_buffer_usage usage;
    int window_id;

    VkDeviceMemory memory; // Vulkan separate memory handle
} canvas_buffer;

canvas_buffer *canvas_buffer_create(int window_id, canvas_buffer_type type, canvas_buffer_usage usage, size_t size, void *initial_data);
void canvas_buffer_destroy(canvas_buffer *buf);

// Update buffer (dynamic buffers)
void canvas_buffer_update(canvas_buffer *buf, void *data, size_t size, size_t offset);

// Get mapped pointer (dynamic buffers)
void *canvas_buffer_map(canvas_buffer *buf);
void canvas_buffer_unmap(canvas_buffer *buf);

// Bind for rendering (draw calls)
void canvas_buffer_bind_vertex(canvas_buffer *buf, uint32_t binding);
void canvas_buffer_bind_index(canvas_buffer *buf);
void canvas_buffer_bind_storage(canvas_buffer *buf, uint32_t binding);
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

Each window also has its own timer in `canvas_info.canvas[window_id].time`.

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

// Maximum number of pointers (mice, touches)
#ifndef CANVAS_POINTER_BUDGET
#define CANVAS_POINTER_BUDGET 10
#endif

// FPS limit for main loop (default: 240)
extern double canvas_limit_mainloop_fps;
```

## Window State

Each window tracks its state in the `canvas_type` structure:
```c
typedef struct {
    int index;           // Window ID
    int64_t x, y;           // Position
    int64_t width, height;  // Size
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
    canvas_cursor_type cursor;     // Current cursor type
} canvas_type;
```

Access via:
```c
extern canvas_type canvas_info.canvas[MAX_CANVAS];
```

## Display Information

Query display information:
```c
typedef struct {
    bool primary;
    int64_t x, y;
    int64_t width, height;
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

## Logging

Define `CANVAS_LOG` enable logging.

Define `CANVAS_LOG_DEBUG` to enable verbose/warning/error/debug logs but keep info logs.

## Example

```c
#include "canvas.h"

void update(int window) {
    canvas_pointer *mouse = canvas_get_pointer(0);
    
    if (mouse && mouse->inside_window) {
        // Change color based on mouse position
        float r = (float)mouse->x / canvas_info.canvas[window].width;
        float g = (float)mouse->y / canvas_info.canvas[window].height;
        canvas_color(window, (float[]){r, g, 0.5f, 1.0f});
        
        // Change cursor on button press
        if (canvas_pointer_pressed(mouse, CANVAS_BUTTON_LEFT)) {
            canvas_cursor(window, CANVAS_CURSOR_HAND);
        }
        if (canvas_pointer_released(mouse, CANVAS_BUTTON_LEFT)) {
            canvas_cursor(window, CANVAS_CURSOR_ARROW);
        }
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
    
    return 0;
}
```

## License
Logos, Branding, Trademarks - Copyright Dawn Larsson 2022

Repository: Apache-2.0 license
