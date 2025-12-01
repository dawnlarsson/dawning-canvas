/*
        Dawning Canvas API

        -  Single File Library  -
        A cross-platform windowing and graphics context API,
        provides the building blocks for creating windows, and setup for the various native graphics APIs.

        -  Window  -
        A window in canvas is just a native window instance, nothing special, use this if you intend to not use
        the graphics API or any other usecases.

        -  Canvas  -
        A canvas is window or view with graphics, and other backend features setup ready to use.
        Using and creating canvases is the intended usecase of this library, it's supposed to get you as fast as possible rendering on screen.
        use the window API if you need a lower level native window cross platform API.

        -  Platform Status  -
        ~ not implemented, x - implemented, \ partially implemented (or wip)

        Platform:               Window  Canvas  Backend     Required Compiler Flags
        Windows                 \       \       DirectX12   -lgdi32 -luser32 -mwindows -ldwmapi -ldxgi -ld3d12 -lwinmm
        MacOS                   \       \       Metal       -framework Cocoa -framework QuartzCore -framework Metal -framework IOKit
        Linux                   \       ~       Vulkan      -ldl -lm
        iOS                     ~       ~       Metal
        Android                 ~       ~       Vulkan
        HTML5                   ~       ~       WebGPU

        **Note for Windows:** Use `x86_64-w64-mingw32-gcc` for cross-compiling to Windows
        **Note for Linux:** Wayland is loaded first, falls back to x11

        -  Info  -
        Canvas is created by Dawn Larsson 2025
        This is licensed under Apache License 2.0, by Dawn Larsson

        https://dawning.dev/  -  https://docs.dawning.dev/

        repo
        https://github.com/dawnlarsson/dawning-canvas

*/

#pragma once

#ifdef __cplusplus
#define CANVAS_EXTERN_C_BEGIN \
    extern "C"                \
    {
#define CANVAS_EXTERN_C_END }
#else
#define CANVAS_EXTERN_C_BEGIN
#define CANVAS_EXTERN_C_END
#endif

CANVAS_EXTERN_C_BEGIN

#ifndef MAX_CANVAS
#define MAX_CANVAS 16
#endif

#ifndef MAX_DISPLAYS
#define MAX_DISPLAYS 8
#endif

#ifndef MAX_CANVAS_TITLE
#define MAX_CANVAS_TITLE 256
#endif

#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#if !defined(_WIN32)
#include <dlfcn.h>
#endif

#ifndef CANVAS_VALIDATION
#define CANVAS_VALIDATION 0
#endif

#if CANVAS_VALIDATION > 0

#include <stdio.h>
#include <assert.h>

#define CANVAS_ERR(...)                                              \
    do                                                               \
    {                                                                \
        fprintf(stderr, "[CANVAS-ERR] %s:%d: ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);                                \
        fflush(stderr);                                              \
    } while (0)

#if CANVAS_VALIDATION >= 2
#define CANVAS_WARN(...)                                              \
    do                                                                \
    {                                                                 \
        fprintf(stderr, "[CANVAS-WARN] %s:%d: ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);                                 \
        fflush(stderr);                                               \
    } while (0)
#else
#define CANVAS_WARN(...) ((void)0)
#endif

#if CANVAS_VALIDATION >= 3
#define CANVAS_INFO(...)                               \
    do                                                 \
    {                                                  \
        fprintf(stdout, "[CANVAS-INFO] " __VA_ARGS__); \
        fflush(stdout);                                \
    } while (0)
#else
#define CANVAS_INFO(...) ((void)0)
#endif

#if CANVAS_VALIDATION >= 4
#define CANVAS_VERBOSE(...)                               \
    do                                                    \
    {                                                     \
        fprintf(stdout, "[CANVAS-VERBOSE] " __VA_ARGS__); \
        fflush(stdout);                                   \
    } while (0)
#else
#define CANVAS_VERBOSE(...) ((void)0)
#endif

#if CANVAS_VALIDATION >= 5
#define CANVAS_DBG(...)                                              \
    do                                                               \
    {                                                                \
        fprintf(stdout, "[CANVAS-DBG] %s:%d: ", __FILE__, __LINE__); \
        fprintf(stdout, __VA_ARGS__);                                \
        fflush(stdout);                                              \
    } while (0)
#else
#define CANVAS_DBG(...) ((void)0)
#endif

#if CANVAS_VALIDATION >= 6
#define CANVAS_TRACE(...)                                                             \
    do                                                                                \
    {                                                                                 \
        fprintf(stdout, "[CANVAS-TRACE] %s:%d %s(): ", __FILE__, __LINE__, __func__); \
        fprintf(stdout, __VA_ARGS__);                                                 \
        fflush(stdout);                                                               \
    } while (0)
#else
#define CANVAS_TRACE(...) ((void)0)
#endif

#if CANVAS_VALIDATION >= 4
#define CANVAS_ASSERT(cond)                              \
    do                                                   \
    {                                                    \
        if (!(cond))                                     \
        {                                                \
            CANVAS_ERR("ASSERTION FAILED: %s\n", #cond); \
            assert(cond);                                \
        }                                                \
    } while (0)
#define CANVAS_ASSERT_MSG(cond, msg)                               \
    do                                                             \
    {                                                              \
        if (!(cond))                                               \
        {                                                          \
            CANVAS_ERR("ASSERTION FAILED: %s - %s\n", #cond, msg); \
            assert(cond);                                          \
        }                                                          \
    } while (0)
#define CANVAS_ASSERT_RANGE(val, min, max)                                                                                                     \
    do                                                                                                                                         \
    {                                                                                                                                          \
        if ((val) < (min) || (val) > (max))                                                                                                    \
        {                                                                                                                                      \
            CANVAS_ERR("RANGE ASSERTION FAILED: %s = %lld not in [%lld, %lld]\n", #val, (long long)(val), (long long)(min), (long long)(max)); \
            assert((val) >= (min) && (val) <= (max));                                                                                          \
        }                                                                                                                                      \
    } while (0)
#define CANVAS_ASSERT_NOT_NULL(ptr)                                  \
    do                                                               \
    {                                                                \
        if ((ptr) == NULL)                                           \
        {                                                            \
            CANVAS_ERR("NULL ASSERTION FAILED: %s is NULL\n", #ptr); \
            assert((ptr) != NULL);                                   \
        }                                                            \
    } while (0)
#define CANVAS_ASSERT_VALID_WINDOW(id)                                        \
    do                                                                        \
    {                                                                         \
        CANVAS_ASSERT_RANGE(id, 0, MAX_CANVAS - 1);                           \
        CANVAS_ASSERT_MSG(canvas_info.canvas[id]._valid, "window not valid"); \
    } while (0)
#define CANVAS_ASSERT_VALID_DISPLAY(id)               \
    do                                                \
    {                                                 \
        CANVAS_ASSERT_RANGE(id, 0, MAX_DISPLAYS - 1); \
    } while (0)
#define CANVAS_ASSERT_POSITIVE(val)                                                            \
    do                                                                                         \
    {                                                                                          \
        if ((val) <= 0)                                                                        \
        {                                                                                      \
            CANVAS_ERR("POSITIVE ASSERTION FAILED: %s = %lld <= 0\n", #val, (long long)(val)); \
            assert((val) > 0);                                                                 \
        }                                                                                      \
    } while (0)
#define CANVAS_ASSERT_NON_NEGATIVE(val)                                                           \
    do                                                                                            \
    {                                                                                             \
        if ((val) < 0)                                                                            \
        {                                                                                         \
            CANVAS_ERR("NON-NEGATIVE ASSERTION FAILED: %s = %lld < 0\n", #val, (long long)(val)); \
            assert((val) >= 0);                                                                   \
        }                                                                                         \
    } while (0)
#define CANVAS_ASSERT_INITIALIZED()                                    \
    do                                                                 \
    {                                                                  \
        CANVAS_ASSERT_MSG(canvas_info.init, "canvas not initialized"); \
    } while (0)
#define CANVAS_ASSERT_GPU_INITIALIZED()                                 \
    do                                                                  \
    {                                                                   \
        CANVAS_ASSERT_MSG(canvas_info.init_gpu, "GPU not initialized"); \
    } while (0)
#else
#define CANVAS_ASSERT(cond) ((void)0)
#define CANVAS_ASSERT_MSG(cond, msg) ((void)0)
#define CANVAS_ASSERT_RANGE(val, min, max) ((void)0)
#define CANVAS_ASSERT_NOT_NULL(ptr) ((void)0)
#define CANVAS_ASSERT_VALID_WINDOW(id) ((void)0)
#define CANVAS_ASSERT_VALID_DISPLAY(id) ((void)0)
#define CANVAS_ASSERT_POSITIVE(val) ((void)0)
#define CANVAS_ASSERT_NON_NEGATIVE(val) ((void)0)
#define CANVAS_ASSERT_INITIALIZED() ((void)0)
#define CANVAS_ASSERT_GPU_INITIALIZED() ((void)0)
#endif

#define CANVAS_ENTER_FUNC() CANVAS_TRACE("ENTER\n")

// Exit tracing at level 5+ (always defined inline based on level)
#if CANVAS_VALIDATION >= 5
#define CANVAS_EXIT_FUNC()                                                                  \
    do                                                                                      \
    {                                                                                       \
        fprintf(stdout, "[CANVAS-TRACE] %s:%d %s(): EXIT\n", __FILE__, __LINE__, __func__); \
        fflush(stdout);                                                                     \
    } while (0)
#define CANVAS_EXIT_FUNC_WITH(val)                                                                                      \
    do                                                                                                                  \
    {                                                                                                                   \
        fprintf(stdout, "[CANVAS-TRACE] %s:%d %s(): EXIT with %lld\n", __FILE__, __LINE__, __func__, (long long)(val)); \
        fflush(stdout);                                                                                                 \
    } while (0)
#elif CANVAS_VALIDATION >= 1
#define CANVAS_EXIT_FUNC() ((void)0)
#define CANVAS_EXIT_FUNC_WITH(val) ((void)0)
#endif

// Return macros - combine exit tracing with return
#define CANVAS_RETURN(val)          \
    do                              \
    {                               \
        CANVAS_EXIT_FUNC_WITH(val); \
        return (val);               \
    } while (0)

#define CANVAS_RETURN_ERR(val, ...) \
    do                              \
    {                               \
        CANVAS_ERR(__VA_ARGS__);    \
        CANVAS_EXIT_FUNC_WITH(val); \
        return (val);               \
    } while (0)

#define CANVAS_RETURN_VOID() \
    do                       \
    {                        \
        CANVAS_EXIT_FUNC();  \
        return;              \
    } while (0)

static void _canvas_validate_all_windows(void);
static void _canvas_validate_all_displays(void);
static void _canvas_validate_all_pointers(void);
static void _canvas_dump_state(void);

#define canvas_pointer_print(id) _canvas_pointer_print_impl(id, __FILE__, __LINE__)

#if CANVAS_VALIDATION >= 5

// Magic canary values to detect memory corruption
#define CANVAS_CANARY_HEAD 0xDEADCAFEBEEFBABEULL
#define CANVAS_CANARY_TAIL 0xFEEDFACEC0FFEE42ULL
#define CANVAS_POISON_BYTE 0xCD
#define CANVAS_FREED_BYTE 0xDD
#define CANVAS_UNINIT_BYTE 0xCC

// Operation sequence tracking
static uint64_t _canvas_op_sequence = 0;
static const char *_canvas_last_op = "none";
static const char *_canvas_last_op_file = "";
static int _canvas_last_op_line = 0;

// Auto-validation frequency (validate every N operations)
#ifndef CANVAS_PARANOID_VALIDATE_FREQ
#define CANVAS_PARANOID_VALIDATE_FREQ 100
#endif

// Track operation and auto-validate periodically
#define CANVAS_PARANOID_OP(name)                                                                  \
    do                                                                                            \
    {                                                                                             \
        _canvas_op_sequence++;                                                                    \
        _canvas_last_op = name;                                                                   \
        _canvas_last_op_file = __FILE__;                                                          \
        _canvas_last_op_line = __LINE__;                                                          \
        CANVAS_DBG("OP #%llu: %s\n", (unsigned long long)_canvas_op_sequence, name);              \
        if ((_canvas_op_sequence % CANVAS_PARANOID_VALIDATE_FREQ) == 0)                           \
        {                                                                                         \
            CANVAS_DBG("Auto-validation at op #%llu\n", (unsigned long long)_canvas_op_sequence); \
            _canvas_paranoid_validate_all();                                                      \
        }                                                                                         \
    } while (0)

// Poison a memory region
#define CANVAS_POISON_MEMORY(ptr, size)                                              \
    do                                                                               \
    {                                                                                \
        if ((ptr) && (size) > 0)                                                     \
        {                                                                            \
            memset((ptr), CANVAS_POISON_BYTE, (size));                               \
            CANVAS_DBG("Poisoned %zu bytes at %p\n", (size_t)(size), (void *)(ptr)); \
        }                                                                            \
    } while (0)

// Mark memory as freed
#define CANVAS_MARK_FREED(ptr, size)                                                     \
    do                                                                                   \
    {                                                                                    \
        if ((ptr) && (size) > 0)                                                         \
        {                                                                                \
            memset((ptr), CANVAS_FREED_BYTE, (size));                                    \
            CANVAS_DBG("Marked freed %zu bytes at %p\n", (size_t)(size), (void *)(ptr)); \
        }                                                                                \
    } while (0)

// Check if memory looks poisoned (returns true if likely corrupted/uninitialized)
static inline bool _canvas_memory_looks_poisoned(const void *ptr, size_t size)
{
    const unsigned char *p = (const unsigned char *)ptr;
    size_t poison_count = 0;
    size_t freed_count = 0;
    for (size_t i = 0; i < size && i < 64; i++)
    {
        if (p[i] == CANVAS_POISON_BYTE)
            poison_count++;
        if (p[i] == CANVAS_FREED_BYTE)
            freed_count++;
    }
    size_t check_size = (size < 64) ? size : 64;
    return (poison_count > check_size / 2) || (freed_count > check_size / 2);
}

// Compute a simple checksum for state validation
static inline uint32_t _canvas_state_checksum(const void *data, size_t len)
{
    const unsigned char *p = (const unsigned char *)data;
    uint32_t hash = 0x811c9dc5; // FNV-1a offset basis
    for (size_t i = 0; i < len; i++)
    {
        hash ^= p[i];
        hash *= 0x01000193; // FNV-1a prime
    }
    return hash;
}

// Forward declarations for paranoid validation (types defined later)
static void _canvas_paranoid_validate_all(void);
static void _canvas_check_canaries(void);

// Assert that a pointer doesn't look like freed/poisoned memory
#define CANVAS_ASSERT_NOT_POISONED(ptr, size)                                        \
    do                                                                               \
    {                                                                                \
        if (_canvas_memory_looks_poisoned((ptr), (size)))                            \
        {                                                                            \
            CANVAS_ERR("POISONED MEMORY DETECTED: %s at %p (size %zu)\n",            \
                       #ptr, (void *)(ptr), (size_t)(size));                         \
            CANVAS_ERR("Last op #%llu: %s at %s:%d\n",                               \
                       (unsigned long long)_canvas_op_sequence,                      \
                       _canvas_last_op, _canvas_last_op_file, _canvas_last_op_line); \
            assert(0 && "poisoned memory detected");                                 \
        }                                                                            \
    } while (0)

// Assert float is valid (not NaN or Inf)
#define CANVAS_ASSERT_VALID_FLOAT(val)                                \
    do                                                                \
    {                                                                 \
        float _v = (val);                                             \
        if (_v != _v || _v == (1.0f / 0.0f) || _v == (-1.0f / 0.0f))  \
        {                                                             \
            CANVAS_ERR("INVALID FLOAT: %s = %f\n", #val, (double)_v); \
            assert(0 && "invalid float (NaN or Inf)");                \
        }                                                             \
    } while (0)

// Assert double is valid
#define CANVAS_ASSERT_VALID_DOUBLE(val)                          \
    do                                                           \
    {                                                            \
        double _v = (val);                                       \
        if (_v != _v || _v == (1.0 / 0.0) || _v == (-1.0 / 0.0)) \
        {                                                        \
            CANVAS_ERR("INVALID DOUBLE: %s = %f\n", #val, _v);   \
            assert(0 && "invalid double (NaN or Inf)");          \
        }                                                        \
    } while (0)

// Check alignment
#define CANVAS_ASSERT_ALIGNED(ptr, align)                                \
    do                                                                   \
    {                                                                    \
        if (((uintptr_t)(ptr) & ((align) - 1)) != 0)                     \
        {                                                                \
            CANVAS_ERR("ALIGNMENT FAILED: %s = %p not aligned to %zu\n", \
                       #ptr, (void *)(ptr), (size_t)(align));            \
            assert(0 && "pointer alignment failed");                     \
        }                                                                \
    } while (0)

// Detect double-free pattern
#define CANVAS_ASSERT_NOT_DOUBLE_FREE(ptr)                                       \
    do                                                                           \
    {                                                                            \
        if ((ptr) && *((unsigned char *)(ptr)) == CANVAS_FREED_BYTE)             \
        {                                                                        \
            CANVAS_ERR("DOUBLE FREE DETECTED: %s at %p\n", #ptr, (void *)(ptr)); \
            assert(0 && "double free detected");                                 \
        }                                                                        \
    } while (0)

#else // CANVAS_VALIDATION < 5

#define CANVAS_PARANOID_OP(name) ((void)0)
#define CANVAS_POISON_MEMORY(ptr, size) ((void)0)
#define CANVAS_MARK_FREED(ptr, size) ((void)0)
#define CANVAS_ASSERT_NOT_POISONED(ptr, size) ((void)0)
#define CANVAS_ASSERT_VALID_FLOAT(val) ((void)0)
#define CANVAS_ASSERT_VALID_DOUBLE(val) ((void)0)
#define CANVAS_ASSERT_ALIGNED(ptr, align) ((void)0)
#define CANVAS_ASSERT_NOT_DOUBLE_FREE(ptr) ((void)0)
#define CANVAS_PARANOID_CHECK() ((void)0)

#endif // CANVAS_VALIDATION >= 5

#else

#define CANVAS_INFO(...)
#define CANVAS_VERBOSE(...)
#define CANVAS_WARN(...)
#define CANVAS_ERR(...)
#define CANVAS_DBG(...)
#define CANVAS_TRACE(...)

#define CANVAS_ASSERT(cond) ((void)0)
#define CANVAS_ASSERT_MSG(cond, msg) ((void)0)
#define CANVAS_ASSERT_RANGE(val, min, max) ((void)0)
#define CANVAS_ASSERT_NOT_NULL(ptr) ((void)0)
#define CANVAS_ASSERT_VALID_WINDOW(id) ((void)0)
#define CANVAS_ASSERT_VALID_DISPLAY(id) ((void)0)
#define CANVAS_ASSERT_POSITIVE(val) ((void)0)
#define CANVAS_ASSERT_NON_NEGATIVE(val) ((void)0)
#define CANVAS_ASSERT_INITIALIZED() ((void)0)
#define CANVAS_ASSERT_GPU_INITIALIZED() ((void)0)
#define CANVAS_ENTER_FUNC() ((void)0)
#define CANVAS_EXIT_FUNC() ((void)0)
#define CANVAS_EXIT_FUNC_WITH(val) ((void)0)
#define CANVAS_RETURN(val) return (val)
#define CANVAS_RETURN_ERR(val, ...) return (val)
#define CANVAS_RETURN_VOID() return

#define canvas_pointer_print(id) ((void)0)

// Paranoid mode stubs for CANVAS_VALIDATION == 0
#define CANVAS_PARANOID_OP(name) ((void)0)
#define CANVAS_POISON_MEMORY(ptr, size) ((void)0)
#define CANVAS_MARK_FREED(ptr, size) ((void)0)
#define CANVAS_ASSERT_NOT_POISONED(ptr, size) ((void)0)
#define CANVAS_ASSERT_VALID_FLOAT(val) ((void)0)
#define CANVAS_ASSERT_VALID_DOUBLE(val) ((void)0)
#define CANVAS_ASSERT_ALIGNED(ptr, align) ((void)0)
#define CANVAS_ASSERT_NOT_DOUBLE_FREE(ptr) ((void)0)
#define CANVAS_PARANOID_CHECK() ((void)0)
static inline void _canvas_init_canaries(void) {}

#endif

typedef void *canvas_library_handle;
typedef void *canvas_window_handle;
typedef void (*canvas_update_callback)(int window);
typedef struct
{
    uint64_t start;
    double current;
    double delta;
    double raw_delta;
    double fps;
    uint64_t frame;
    double accumulator;
    double alpha;
    double last;
    double times[60];
    int frame_index;
} canvas_time_data;

#ifndef CANVAS_POINTER_SAMPLE_FRAMES
#define CANVAS_POINTER_SAMPLE_FRAMES 8
#endif

#ifndef CANVAS_POINTER_BUDGET
#define CANVAS_POINTER_BUDGET 10
#endif

typedef enum
{
    CANVAS_CURSOR_HIDDEN,
    CANVAS_CURSOR_ARROW,
    CANVAS_CURSOR_TEXT,
    CANVAS_CURSOR_CROSSHAIR,
    CANVAS_CURSOR_HAND,
    CANVAS_CURSOR_SIZE_NS,
    CANVAS_CURSOR_SIZE_EW,
    CANVAS_CURSOR_SIZE_NESW,
    CANVAS_CURSOR_SIZE_NWSE,
    CANVAS_CURSOR_SIZE_ALL,
    CANVAS_CURSOR_NOT_ALLOWED,
    CANVAS_CURSOR_WAIT,
} canvas_cursor_type;

typedef enum
{
    CANVAS_POINTER_NONE = 0,
    CANVAS_POINTER_MOUSE = 1,
    CANVAS_POINTER_TOUCH = 2,
    CANVAS_POINTER_PEN = 3,
} canvas_pointer_type;

typedef enum
{
    CANVAS_BUTTON_LEFT = (1 << 0),
    CANVAS_BUTTON_RIGHT = (1 << 1),
    CANVAS_BUTTON_MIDDLE = (1 << 2),
    CANVAS_BUTTON_X1 = (1 << 3),
    CANVAS_BUTTON_X2 = (1 << 4),
} canvas_pointer_button;

typedef struct
{
    int64_t x, y;
    double time;
} canvas_pointer_sample;

typedef struct
{
#if CANVAS_VALIDATION >= 5
    uint64_t _canary_head;
#endif
    int id;
    canvas_pointer_type type;
    int window_id;

    int64_t x, y;
    int64_t screen_x, screen_y;
    int display;

    uint32_t buttons;
    uint32_t buttons_pressed;
    uint32_t buttons_released;

    float scroll_x, scroll_y;
    float pressure;

    bool active;
    bool inside_window;
    bool captured;
    bool relative_mode;

    canvas_pointer_sample _samples[CANVAS_POINTER_SAMPLE_FRAMES];
    int _sample_index;

    canvas_cursor_type cursor;
#if CANVAS_VALIDATION >= 5
    uint64_t _canary_tail;
#endif
} canvas_pointer;

canvas_pointer *canvas_get_pointer(int id);
int canvas_get_active_pointers(canvas_pointer **out);

bool canvas_pointer_down(canvas_pointer *p, canvas_pointer_button btn);
bool canvas_pointer_pressed(canvas_pointer *p, canvas_pointer_button btn);
bool canvas_pointer_released(canvas_pointer *p, canvas_pointer_button btn);

float canvas_pointer_velocity(canvas_pointer *p);
float canvas_pointer_direction(canvas_pointer *p);
void canvas_pointer_delta(canvas_pointer *p, int64_t *dx, int64_t *dy);

void canvas_pointer_capture(int window_id);
void canvas_pointer_release();

int canvas(int64_t x, int64_t y, int64_t width, int64_t height, const char *title);
int canvas_window(int64_t x, int64_t y, int64_t width, int64_t height, const char *title);

int canvas_startup();
int canvas_color(int window, const float color[4]);
int canvas_set(int window_id, int display, int64_t x, int64_t y, int64_t width, int64_t height, const char *title);

int canvas_minimize(int window);
int canvas_maximize(int window);
int canvas_fullscreen(int window);
int canvas_restore(int window);
int canvas_close(int window);

int canvas_run(canvas_update_callback update);
int canvas_set_update_callback(int window, canvas_update_callback callback);
void canvas_limit_fps(canvas_time_data *time, double target_fps);
void canvas_sleep(double seconds);

int canvas_exit();

void canvas_time_init(canvas_time_data *time);
void canvas_time_update(canvas_time_data *time);
double canvas_get_time(canvas_time_data *time);
int canvas_time_fixed_step(canvas_time_data *time, double fixed_dt, int max_steps);
int canvas_cursor(int window_id, canvas_cursor_type cursor);

typedef enum
{
    CANVAS_BUFFER_VERTEX,
    CANVAS_BUFFER_INDEX,
    CANVAS_BUFFER_UNIFORM,
    CANVAS_BUFFER_STORAGE,
} canvas_buffer_type;

typedef enum
{
    CANVAS_BUFFER_STATIC,
    CANVAS_BUFFER_DYNAMIC,
    CANVAS_BUFFER_STAGING,
} canvas_buffer_usage;

typedef struct
{
    void *platform_handle;
    void *mapped;
    size_t size;
    canvas_buffer_type type;
    canvas_buffer_usage usage;
    int window_id;

    uint64_t memory;
} canvas_buffer;

canvas_buffer *canvas_buffer_create(int window_id, canvas_buffer_type type, canvas_buffer_usage usage, size_t size, void *initial_data);
void canvas_buffer_destroy(canvas_buffer *buf);

void canvas_buffer_update(canvas_buffer *buf, void *data, size_t size, size_t offset);

void *canvas_buffer_map(canvas_buffer *buf);
void canvas_buffer_unmap(canvas_buffer *buf);

void canvas_buffer_bind_vertex(canvas_buffer *buf, uint32_t binding);
void canvas_buffer_bind_index(canvas_buffer *buf);
void canvas_buffer_bind_storage(canvas_buffer *buf, uint32_t binding);

void canvas_main_loop();
int _canvas_platform();
int _canvas_update();
int _canvas_window(int64_t x, int64_t y, int64_t width, int64_t height, const char *title);
int _canvas_gpu_init();
int _canvas_gpu_new_window(int window_id);
int _canvas_window_resize(int window_id);
int _canvas_primary_display_index();

typedef struct
{
    float m[16];
} canvas_mat4;

typedef struct
{
    float x, y, z;
} canvas_vec3;

typedef struct
{
#if CANVAS_VALIDATION >= 5
    uint64_t _canary_head;
#endif
    int index, display;
    int64_t x, y, width, height;

    bool resize, close, titlebar, os_moved, os_resized;
    bool minimized, maximized, fullscreen, vsync, _valid;

    float clear[4];
    char title[MAX_CANVAS_TITLE];
    canvas_window_handle window;
    canvas_update_callback update;
    canvas_time_data time;
    canvas_cursor_type cursor, active_cursor;

    canvas_mat4 view_matrix;
    canvas_mat4 projection_matrix;
    void *depth_texture;
#if CANVAS_VALIDATION >= 5
    uint64_t _canary_tail;
#endif
} canvas_type;

typedef struct
{
    bool primary;
    int64_t x, y, width, height;
    float scale;
    int refresh_rate;
} canvas_display;

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
#define CANVAS_ERR_INVALID_SIZE -37

typedef enum KEY
{
    KEY_ERROR_ROLLOVER = 1,
    KEY_POST_FAIL = 2,
    KEY_ERROR_UNDEFINED = 3,

    KEY_A = 4,
    KEY_B = 5,
    KEY_C = 6,
    KEY_D = 7,
    KEY_E = 8,
    KEY_F = 9,
    KEY_G = 10,
    KEY_H = 11,
    KEY_I = 12,
    KEY_J = 13,
    KEY_K = 14,
    KEY_L = 15,
    KEY_M = 16,
    KEY_N = 17,
    KEY_O = 18,
    KEY_P = 19,
    KEY_Q = 20,
    KEY_R = 21,
    KEY_S = 22,
    KEY_T = 23,
    KEY_U = 24,
    KEY_V = 25,
    KEY_W = 26,
    KEY_X = 27,
    KEY_Y = 28,
    KEY_Z = 29,

    KEY_1_EXCLAMATION = 30,
    KEY_2_AT = 31,
    KEY_3_HASH = 32,
    KEY_4_DOLLAR = 33,
    KEY_5_PERCENT = 34,
    KEY_6_CARET = 35,
    KEY_7_AMPERSAND = 36,
    KEY_8_ASTERISK = 37,
    KEY_9_LEFT_PAREN = 38,
    KEY_0_RIGHT_PAREN = 39,

    KEY_RETURN_ENTER = 40,
    KEY_ESCAPE = 41,
    KEY_DELETE_BACKSPACE = 42,
    KEY_TAB = 43,
    KEY_SPACEBAR = 44,
    KEY_MINUS_UNDERSCORE = 45,
    KEY_EQUAL_PLUS = 46,
    KEY_LEFT_BRACKET_BRACE = 47,
    KEY_RIGHT_BRACKET_BRACE = 48,
    KEY_BACKSLASH_PIPE = 49,
    KEY_NON_US_HASH_TILDE = 50,
    KEY_SEMICOLON_COLON = 51,
    KEY_APOSTROPHE_DOUBLE_QUOTE = 52,
    KEY_GRAVE_ACCENT_TILDE = 53,
    KEY_COMMA_LESS = 54,
    KEY_PERIOD_GREATER = 55,
    KEY_SLASH_QUESTION = 56,

    KEY_CAPS_LOCK = 57,

    KEY_F1 = 58,
    KEY_F2 = 59,
    KEY_F3 = 60,
    KEY_F4 = 61,
    KEY_F5 = 62,
    KEY_F6 = 63,
    KEY_F7 = 64,
    KEY_F8 = 65,
    KEY_F9 = 66,
    KEY_F10 = 67,
    KEY_F11 = 68,
    KEY_F12 = 69,

    KEY_PRINT_SCREEN = 70,
    KEY_SCROLL_LOCK = 71,
    KEY_PAUSE = 72,
    KEY_INSERT = 73,
    KEY_HOME = 74,
    KEY_PAGE_UP = 75,
    KEY_DELETE_FORWARD = 76,
    KEY_END = 77,
    KEY_PAGE_DOWN = 78,
    KEY_RIGHT_ARROW = 79,
    KEY_LEFT_ARROW = 80,
    KEY_DOWN_ARROW = 81,
    KEY_UP_ARROW = 82,

    KEYPAD_NUM_LOCK_CLEAR = 83,
    KEYPAD_SLASH = 84,
    KEYPAD_ASTERISK = 85,
    KEYPAD_MINUS = 86,
    KEYPAD_PLUS = 87,
    KEYPAD_ENTER = 88,
    KEYPAD_1_END = 89,
    KEYPAD_2_DOWN = 90,
    KEYPAD_3_PAGEDOWN = 91,
    KEYPAD_4_LEFT = 92,
    KEYPAD_5 = 93,
    KEYPAD_6_RIGHT = 94,
    KEYPAD_7_HOME = 95,
    KEYPAD_8_UP = 96,
    KEYPAD_9_PAGEUP = 97,
    KEYPAD_0_INSERT = 98,
    KEYPAD_DOT_DELETE = 99,

    KEY_NON_US_BACKSLASH_PIPE = 100,
    KEY_APPLICATION = 101,
    KEY_POWER = 102,
    KEYPAD_EQUAL = 103,
    KEY_F13 = 104,
    KEY_F14 = 105,
    KEY_F15 = 106,
    KEY_F16 = 107,
    KEY_F17 = 108,
    KEY_F18 = 109,
    KEY_F19 = 110,
    KEY_F20 = 111,
    KEY_F21 = 112,
    KEY_F22 = 113,
    KEY_F23 = 114,
    KEY_F24 = 115,

    KEY_EXECUTE = 116,
    KEY_HELP = 117,
    KEY_MENU = 118,
    KEY_SELECT = 119,
    KEY_STOP = 120,
    KEY_AGAIN = 121,
    KEY_UNDO = 122,
    KEY_CUT = 123,
    KEY_COPY = 124,
    KEY_PASTE = 125,
    KEY_FIND = 126,
    KEY_MUTE = 127,
    KEY_VOLUME_UP = 128,
    KEY_VOLUME_DOWN = 129,

    KEY_LOCKING_CAPS_LOCK = 130,
    KEY_LOCKING_NUM_LOCK = 131,
    KEY_LOCKING_SCROLL_LOCK = 132,

    KEYPAD_COMMA = 133,
    KEYPAD_EQUAL_SIGN = 134,

    KEY_INTERNATIONAL1 = 135,
    KEY_INTERNATIONAL2 = 136,
    KEY_INTERNATIONAL3 = 137,
    KEY_INTERNATIONAL5_1 = 138,
    KEY_INTERNATIONAL5_2 = 139,
    KEY_INTERNATIONAL6 = 140,
    KEY_INTERNATIONAL7 = 141,
    KEY_INTERNATIONAL8 = 142,
    KEY_INTERNATIONAL9 = 143,

    KEY_LANG1 = 144,
    KEY_LANG2 = 145,
    KEY_LANG3 = 146,
    KEY_LANG4 = 147,
    KEY_LANG5 = 148,
    KEY_LANG6 = 149,
    KEY_LANG7 = 150,
    KEY_LANG8 = 151,
    KEY_LANG9 = 152,

    KEY_ALTERNATE_ERASE = 153,
    KEY_SYSREQ_ATTENTION = 154,
    KEY_CANCEL = 155,
    KEY_CLEAR = 156,
    KEY_PRIOR = 157,
    KEY_RETURN = 158,
    KEY_SEPARATOR = 159,
    KEY_OUT = 160,
    KEY_OPER = 161,
    KEY_CLEAR_AGAIN = 162,
    KEY_CRSEL_PROPS = 163,
    KEY_EXSEL = 164,

    KEYPAD_00 = 176,
    KEYPAD_000 = 177,
    KEYPAD_THOUSANDS_SEPARATOR = 178,
    KEYPAD_DECIMAL_SEPARATOR = 179,
    KEYPAD_CURRENCY_UNIT = 180,
    KEYPAD_CURRENCY_SUB_UNIT = 181,
    KEYPAD_LEFT_PAREN = 182,
    KEYPAD_RIGHT_PAREN = 183,
    KEYPAD_LEFT_BRACE = 184,
    KEYPAD_RIGHT_BRACE = 185,
    KEYPAD_TAB = 186,
    KEYPAD_BACKSPACE = 187,
    KEYPAD_A = 188,
    KEYPAD_B = 189,
    KEYPAD_C = 190,
    KEYPAD_D = 191,
    KEYPAD_E = 192,
    KEYPAD_F = 193,
    KEYPAD_XOR = 194,
    KEYPAD_LOGICAL_AND = 195,
    KEYPAD_PERCENT = 196,
    KEYPAD_LESS = 197,
    KEYPAD_GREATER = 198,
    KEYPAD_AMPERSAND = 199,
    KEYPAD_DOUBLE_AMPERSAND = 200,
    KEYPAD_PIPE = 201,
    KEYPAD_DOUBLE_PIPE = 202,
    KEYPAD_COLON = 203,
    KEYPAD_HASH = 204,
    KEYPAD_SPACE = 205,
    KEYPAD_AT = 206,
    KEYPAD_EXCLAMATION = 207,
    KEYPAD_MEMORY_STORE = 208,
    KEYPAD_MEMORY_RECALL = 209,
    KEYPAD_MEMORY_CLEAR = 210,
    KEYPAD_MEMORY_ADD = 211,
    KEYPAD_MEMORY_SUBTRACT = 212,
    KEYPAD_MEMORY_MULTIPLY = 213,
    KEYPAD_MEMORY_DIVIDE = 214,
    KEYPAD_PLUS_MINUS = 215,
    KEYPAD_CLEAR = 216,
    KEYPAD_CLEAR_ENTRY = 217,
    KEYPAD_BINARY = 218,
    KEYPAD_OCTAL = 219,
    KEYPAD_DECIMAL = 220,
    KEYPAD_HEXADECIMAL = 221,

    KEY_LEFT_CONTROL = 224,
    KEY_LEFT_SHIFT = 225,
    KEY_LEFT_ALT = 226,
    KEY_LEFT_GUI = 227,
    KEY_RIGHT_CONTROL = 228,
    KEY_RIGHT_SHIFT = 229,
    KEY_RIGHT_ALT = 230,
    KEY_RIGHT_GUI = 231,
} KEY;

static const unsigned char win32_to_hid[256] = {
    0, 41, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 45, 46, 42, 43, 20, 26, 8, 21, 23, 28, 24, 12, 18, 19, 47, 48, 40, 224, 4, 22, 7,
    9, 10, 11, 13, 14, 15, 51, 52, 53, 225, 49, 29, 27, 6, 25, 5, 17, 16, 54, 55, 56, 229, 85, 226, 44, 57, 58, 59, 60, 61, 62, 63, 64,
    65, 66, 67, 83, 71, 95, 96, 97, 86, 92, 93, 94, 87, 89, 90, 91, 98, 99, 0, 0, 0, 68, 69,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static const unsigned char x11_to_hid[248] = {
    0, 41, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 45, 46, 42, 43, 20, 26, 8, 21, 23, 28, 24, 12, 18, 19, 47, 48, 158, 224, 4, 22,
    7, 9, 10, 11, 13, 14, 15, 51, 52, 53, 225, 49, 29, 27, 6, 25, 5, 17, 16, 54, 55, 56, 229, 226, 44, 57, 58, 59, 60, 61, 62,
    63, 64, 65, 66, 67, 83, 71, 95, 96, 97, 86, 92, 93, 94, 87, 89, 90, 91, 98, 99, 0, 0, 68, 69, 0, 0, 135, 0, 0, 0, 0, 0,
    70, 104, 111, 107, 113, 106, 105, 108, 109, 110, 112, 118, 115, 0, 117, 102, 0, 119, 0, 103, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

typedef struct
{
    bool keys[256];
    bool keys_pressed[256];
    bool keys_released[256];
} canvas_keyboard_state;

canvas_keyboard_state canvas_keyboard = {0};

inline bool canvas_key_down(int key)
{
    CANVAS_ASSERT_RANGE(key, 0, 255);
#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT_NOT_POISONED(&canvas_keyboard, sizeof(canvas_keyboard));
#endif
    if (key < 0 || key >= 256)
        return false;
    return canvas_keyboard.keys[key];
}

inline bool canvas_key_pressed(int key)
{
    CANVAS_ASSERT_RANGE(key, 0, 255);
#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT_NOT_POISONED(&canvas_keyboard, sizeof(canvas_keyboard));
#endif
    if (key < 0 || key >= 256)
        return false;
    return canvas_keyboard.keys_pressed[key];
}

inline bool canvas_key_released(int key)
{
    CANVAS_ASSERT_RANGE(key, 0, 255);
#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT_NOT_POISONED(&canvas_keyboard, sizeof(canvas_keyboard));
#endif
    if (key < 0 || key >= 256)
        return false;
    return canvas_keyboard.keys_released[key];
}

#ifndef CANVAS_NO_EASY_API

inline bool key_down(int key)
{
    return canvas_key_down(key);
}

inline bool key_press(int key)
{
    return canvas_key_pressed(key);
}

inline bool key_up(int key)
{
    return canvas_key_released(key);
}

inline bool pointer_down(canvas_pointer *p, canvas_pointer_button btn)
{
    return canvas_pointer_down(p, btn);
}

inline bool pointer_press(canvas_pointer *p, canvas_pointer_button btn)
{
    return canvas_pointer_pressed(p, btn);
}

inline bool pointer_up(canvas_pointer *p, canvas_pointer_button btn)
{
    return canvas_pointer_released(p, btn);
}

inline float pointer_vel(canvas_pointer *p)
{
    return canvas_pointer_velocity(p);
}

inline float pointer_dir(canvas_pointer *p)
{
    return canvas_pointer_direction(p);
}

#endif

#ifndef CANVAS_HEADER_ONLY

typedef struct
{
#if CANVAS_VALIDATION >= 5
    uint64_t _canary_head;
#endif
    bool init, init_gpu, init_post, os_timed, auto_exit, quit, display_changed;
    int display_count, limit_fps, highest_refresh_rate;

    canvas_type canvas[MAX_CANVAS];
    canvas_display display[MAX_DISPLAYS];

    canvas_update_callback update_callback;
    canvas_time_data time;

    int pointer_count;
    canvas_pointer pointers[CANVAS_POINTER_BUDGET];

#if CANVAS_VALIDATION >= 5
    uint64_t _canary_tail;
    uint32_t _state_checksum;    // Checksum of critical state
    uint64_t _last_validated_op; // Last operation number when validated
#endif
} canvas_context_type;

canvas_context_type canvas_info = {0};

#if CANVAS_VALIDATION > 0
static void _canvas_pointer_print_impl(int id, const char *file, int line)
{
    printf("\033c");
    puts("\033[H");

    canvas_pointer *p = canvas_get_pointer(id);

    if (!p)
    {
        printf("pointer %d: not found or active\n", id);
        return;
    }

    printf("=== POINTER DEBUG (from %s:%d) ===\n\n", file, line);

    printf("Identity:\n");
    printf("  ID:              %d\n", p->id);
    printf("  Type:            %s\n",
           p->type == CANVAS_POINTER_MOUSE ? "MOUSE" : p->type == CANVAS_POINTER_TOUCH ? "TOUCH"
                                                   : p->type == CANVAS_POINTER_PEN     ? "PEN"
                                                                                       : "UNKNOWN");
    printf("  Window ID:       %d\n", p->window_id);
    printf("  Active:          %s\n", p->active ? "YES" : "NO");
    printf("\n");

    printf("Position:\n");
    printf("  Window:          (%" PRId64 ", %" PRId64 ")\n", p->x, p->y);
    printf("  Screen:          (%" PRId64 ", %" PRId64 ")\n", p->screen_x, p->screen_y);
    printf("  Display:         %d\n", p->display);
    printf("  Inside Window:   %s\n", p->inside_window ? "YES" : "NO");
    printf("\n");

    printf("State:\n");
    printf("  Captured:        %s\n", p->captured ? "YES" : "NO");
    printf("  Relative Mode:   %s\n", p->relative_mode ? "YES" : "NO");
    printf("  Pressure:        %.2f\n", p->pressure);
    printf("\n");

    printf("Buttons:\n");
    printf("  Current:         0x%04X [", p->buttons);
    if (p->buttons & CANVAS_BUTTON_LEFT)
        printf("L");
    if (p->buttons & CANVAS_BUTTON_RIGHT)
        printf("R");
    if (p->buttons & CANVAS_BUTTON_MIDDLE)
        printf("M");
    if (p->buttons & CANVAS_BUTTON_X1)
        printf("X1");
    if (p->buttons & CANVAS_BUTTON_X2)
        printf("X2");
    printf("]\n");

    printf("  Pressed (frame): 0x%04X [", p->buttons_pressed);
    if (p->buttons_pressed & CANVAS_BUTTON_LEFT)
        printf("L");
    if (p->buttons_pressed & CANVAS_BUTTON_RIGHT)
        printf("R");
    if (p->buttons_pressed & CANVAS_BUTTON_MIDDLE)
        printf("M");
    if (p->buttons_pressed & CANVAS_BUTTON_X1)
        printf("X1");
    if (p->buttons_pressed & CANVAS_BUTTON_X2)
        printf("X2");
    printf("]\n");

    printf("  Released (frame): 0x%04X [", p->buttons_released);
    if (p->buttons_released & CANVAS_BUTTON_LEFT)
        printf("L");
    if (p->buttons_released & CANVAS_BUTTON_RIGHT)
        printf("R");
    if (p->buttons_released & CANVAS_BUTTON_MIDDLE)
        printf("M");
    if (p->buttons_released & CANVAS_BUTTON_X1)
        printf("X1");
    if (p->buttons_released & CANVAS_BUTTON_X2)
        printf("X2");
    printf("]\n");
    printf("\n");

    printf("Scroll (this frame):\n");
    printf("  X: %+.2f    Y: %+.2f\n", p->scroll_x, p->scroll_y);
    printf("\n");

    printf("Cursor:\n");
    printf("  Type:            %s\n",
           p->cursor == CANVAS_CURSOR_HIDDEN ? "HIDDEN" : p->cursor == CANVAS_CURSOR_ARROW     ? "ARROW"
                                                      : p->cursor == CANVAS_CURSOR_TEXT        ? "TEXT"
                                                      : p->cursor == CANVAS_CURSOR_CROSSHAIR   ? "CROSSHAIR"
                                                      : p->cursor == CANVAS_CURSOR_HAND        ? "HAND"
                                                      : p->cursor == CANVAS_CURSOR_SIZE_NS     ? "SIZE_NS"
                                                      : p->cursor == CANVAS_CURSOR_SIZE_EW     ? "SIZE_EW"
                                                      : p->cursor == CANVAS_CURSOR_SIZE_NESW   ? "SIZE_NESW"
                                                      : p->cursor == CANVAS_CURSOR_SIZE_NWSE   ? "SIZE_NWSE"
                                                      : p->cursor == CANVAS_CURSOR_SIZE_ALL    ? "SIZE_ALL"
                                                      : p->cursor == CANVAS_CURSOR_NOT_ALLOWED ? "NOT_ALLOWED"
                                                      : p->cursor == CANVAS_CURSOR_WAIT        ? "WAIT"
                                                                                               : "UNKNOWN");
    printf("\n");

    int newest = (p->_sample_index - 1 + CANVAS_POINTER_SAMPLE_FRAMES) % CANVAS_POINTER_SAMPLE_FRAMES;
    int oldest = p->_sample_index;

    canvas_pointer_sample *s_new = &p->_samples[newest];
    canvas_pointer_sample *s_old = &p->_samples[oldest];

    double dt = s_new->time - s_old->time;

    if (dt > 0.001)
    {
        int dx = s_new->x - s_old->x;
        int dy = s_new->y - s_old->y;
        float distance = sqrtf((float)(dx * dx + dy * dy));
        float velocity = distance / (float)dt;
        float direction = atan2f((float)dy, (float)dx) * 180.0f / 3.14159265f;

        printf("Motion Analysis:\n");
        printf("  Velocity:        %.1f px/s\n", velocity);
        printf("  Direction:       %.1f°\n", direction);
        printf("  Delta:           (%+d, %+d) px\n", dx, dy);
        printf("  Sample Period:   %.3f ms\n", dt * 1000.0);
    }
    else
    {
        printf("Motion Analysis:\n");
        printf("  (insufficient sample data)\n");
    }
    printf("\n");

    printf("Sample History (ring buffer):\n");
    printf("  Current Index:   %d / %d\n", p->_sample_index, CANVAS_POINTER_SAMPLE_FRAMES);
    printf("  Samples:\n");

    for (int i = 0; i < CANVAS_POINTER_SAMPLE_FRAMES; i++)
    {
        int idx = (p->_sample_index + i) % CANVAS_POINTER_SAMPLE_FRAMES;
        canvas_pointer_sample *s = &p->_samples[idx];

        char marker = (idx == newest) ? '>' : (idx == p->_sample_index) ? 'O'
                                                                        : ' ';

        if (s->time > 0.0)
        {
            printf("  %c [%d] (%4" PRId64 ", %4" PRId64 ") @ %.6fs\n",
                   marker, idx, s->x, s->y, s->time);
        }
        else
        {
            printf("    [%d] (empty)\n", idx);
        }
    }
}

static void _canvas_validate_all_windows(void)
{
    CANVAS_TRACE("Validating all windows\n");
    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (canvas_info.canvas[i]._valid)
        {
            CANVAS_ASSERT_NOT_NULL(canvas_info.canvas[i].window);
            CANVAS_ASSERT_RANGE(canvas_info.canvas[i].display, 0, MAX_DISPLAYS - 1);
            CANVAS_ASSERT(canvas_info.canvas[i].width >= 0);
            CANVAS_ASSERT(canvas_info.canvas[i].height >= 0);
            CANVAS_ASSERT_RANGE(canvas_info.canvas[i].cursor, CANVAS_CURSOR_HIDDEN, CANVAS_CURSOR_WAIT);
            CANVAS_DBG("Window %d: valid, size=%" PRId64 "x%" PRId64 ", display=%d\n",
                       i, canvas_info.canvas[i].width, canvas_info.canvas[i].height, canvas_info.canvas[i].display);
        }
    }
}

static void _canvas_validate_all_displays(void)
{
    CANVAS_TRACE("Validating all displays\n");
    CANVAS_ASSERT_RANGE(canvas_info.display_count, 0, MAX_DISPLAYS);
    for (int i = 0; i < canvas_info.display_count; i++)
    {
        CANVAS_ASSERT(canvas_info.display[i].width > 0);
        CANVAS_ASSERT(canvas_info.display[i].height > 0);
        CANVAS_ASSERT(canvas_info.display[i].refresh_rate > 0);
        CANVAS_DBG("Display %d: %" PRId64 "x%" PRId64 " @ %dHz, primary=%d\n",
                   i, canvas_info.display[i].width, canvas_info.display[i].height,
                   canvas_info.display[i].refresh_rate, canvas_info.display[i].primary);
    }
}

static void _canvas_validate_all_pointers(void)
{
    CANVAS_TRACE("Validating all pointers\n");
    CANVAS_ASSERT_RANGE(canvas_info.pointer_count, 0, CANVAS_POINTER_BUDGET);
    for (int i = 0; i < canvas_info.pointer_count; i++)
    {
        if (canvas_info.pointers[i].active)
        {
            CANVAS_ASSERT_RANGE(canvas_info.pointers[i].id, 0, CANVAS_POINTER_BUDGET - 1);
        }
    }
}

static void _canvas_dump_state(void)
{
    CANVAS_INFO("=== CANVAS STATE DUMP ===\n");
    CANVAS_INFO("init=%d, init_gpu=%d, init_post=%d\n",
                canvas_info.init, canvas_info.init_gpu, canvas_info.init_post);
    CANVAS_INFO("display_count=%d, pointer_count=%d\n",
                canvas_info.display_count, canvas_info.pointer_count);
    CANVAS_INFO("quit=%d, auto_exit=%d\n", canvas_info.quit, canvas_info.auto_exit);

    _canvas_validate_all_windows();
    _canvas_validate_all_displays();
    _canvas_validate_all_pointers();

    CANVAS_INFO("=== END STATE DUMP ===\n");
}

#if CANVAS_VALIDATION >= 5

static void _canvas_init_canaries(void)
{
    CANVAS_DBG("Initializing canaries for paranoid mode\n");

    // Initialize context canaries
    canvas_info._canary_head = CANVAS_CANARY_HEAD;
    canvas_info._canary_tail = CANVAS_CANARY_TAIL;
    canvas_info._state_checksum = 0;
    canvas_info._last_validated_op = 0;

    // Initialize window canaries
    for (int i = 0; i < MAX_CANVAS; i++)
    {
        canvas_info.canvas[i]._canary_head = CANVAS_CANARY_HEAD;
        canvas_info.canvas[i]._canary_tail = CANVAS_CANARY_TAIL;
    }

    // Initialize pointer canaries
    for (int i = 0; i < CANVAS_POINTER_BUDGET; i++)
    {
        canvas_info.pointers[i]._canary_head = CANVAS_CANARY_HEAD;
        canvas_info.pointers[i]._canary_tail = CANVAS_CANARY_TAIL;
    }

    CANVAS_DBG("Canaries initialized: HEAD=0x%llX TAIL=0x%llX\n",
               (unsigned long long)CANVAS_CANARY_HEAD,
               (unsigned long long)CANVAS_CANARY_TAIL);
}

static void _canvas_check_canaries(void)
{
    // CANVAS_DBG("Checking canaries...\n");

    // Check context canaries
    if (canvas_info._canary_head != CANVAS_CANARY_HEAD)
    {
        CANVAS_ERR("CANARY CORRUPTION: canvas_info._canary_head = 0x%llX (expected 0x%llX)\n",
                   (unsigned long long)canvas_info._canary_head,
                   (unsigned long long)CANVAS_CANARY_HEAD);
        CANVAS_ERR("Memory corruption detected BEFORE canvas_info structure!\n");
        assert(0 && "canary corruption (head)");
    }
    if (canvas_info._canary_tail != CANVAS_CANARY_TAIL)
    {
        CANVAS_ERR("CANARY CORRUPTION: canvas_info._canary_tail = 0x%llX (expected 0x%llX)\n",
                   (unsigned long long)canvas_info._canary_tail,
                   (unsigned long long)CANVAS_CANARY_TAIL);
        CANVAS_ERR("Memory corruption detected AFTER canvas_info structure!\n");
        assert(0 && "canary corruption (tail)");
    }

    // Check window canaries
    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (canvas_info.canvas[i]._canary_head != CANVAS_CANARY_HEAD)
        {
            CANVAS_ERR("CANARY CORRUPTION: canvas[%d]._canary_head = 0x%llX\n",
                       i, (unsigned long long)canvas_info.canvas[i]._canary_head);
            assert(0 && "window canary corruption (head)");
        }
        if (canvas_info.canvas[i]._canary_tail != CANVAS_CANARY_TAIL)
        {
            CANVAS_ERR("CANARY CORRUPTION: canvas[%d]._canary_tail = 0x%llX\n",
                       i, (unsigned long long)canvas_info.canvas[i]._canary_tail);
            assert(0 && "window canary corruption (tail)");
        }
    }

    // Check pointer canaries
    for (int i = 0; i < CANVAS_POINTER_BUDGET; i++)
    {
        if (canvas_info.pointers[i]._canary_head != CANVAS_CANARY_HEAD)
        {
            CANVAS_ERR("CANARY CORRUPTION: pointers[%d]._canary_head = 0x%llX\n",
                       i, (unsigned long long)canvas_info.pointers[i]._canary_head);
            assert(0 && "pointer canary corruption (head)");
        }
        if (canvas_info.pointers[i]._canary_tail != CANVAS_CANARY_TAIL)
        {
            CANVAS_ERR("CANARY CORRUPTION: pointers[%d]._canary_tail = 0x%llX\n",
                       i, (unsigned long long)canvas_info.pointers[i]._canary_tail);
            assert(0 && "pointer canary corruption (tail)");
        }
    }

    // CANVAS_DBG("All canaries OK\n");
}

static void _canvas_validate_pointer_sanity(canvas_pointer *p)
{
    if (!p)
        return;

    CANVAS_ASSERT_NOT_POISONED(p, sizeof(canvas_pointer));

    // Validate floats aren't NaN/Inf
    CANVAS_ASSERT_VALID_FLOAT(p->scroll_x);
    CANVAS_ASSERT_VALID_FLOAT(p->scroll_y);
    CANVAS_ASSERT_VALID_FLOAT(p->pressure);

    // Validate sample times
    for (int i = 0; i < CANVAS_POINTER_SAMPLE_FRAMES; i++)
    {
        CANVAS_ASSERT_VALID_DOUBLE(p->_samples[i].time);
    }

    // Validate sample index is in bounds
    CANVAS_ASSERT_RANGE(p->_sample_index, 0, CANVAS_POINTER_SAMPLE_FRAMES - 1);

    // Validate type enum
    CANVAS_ASSERT_RANGE(p->type, CANVAS_POINTER_NONE, CANVAS_POINTER_PEN);

    // Validate cursor enum
    CANVAS_ASSERT_RANGE(p->cursor, CANVAS_CURSOR_HIDDEN, CANVAS_CURSOR_WAIT);

    // Validate pressure is reasonable (0.0 to 1.0, allow slight overflow)
    if (p->pressure < -0.1f || p->pressure > 1.1f)
    {
        CANVAS_WARN("Suspicious pressure value: %f (pointer %d)\n", p->pressure, p->id);
    }
}

static void _canvas_validate_window_sanity(int id)
{
    if (id < 0 || id >= MAX_CANVAS)
        return;

    canvas_type *w = &canvas_info.canvas[id];

    CANVAS_ASSERT_NOT_POISONED(w, sizeof(canvas_type));

    if (!w->_valid)
        return;

    // Validate floats in clear color
    for (int i = 0; i < 4; i++)
    {
        CANVAS_ASSERT_VALID_FLOAT(w->clear[i]);
        if (w->clear[i] < 0.0f || w->clear[i] > 1.0f)
        {
            CANVAS_WARN("Suspicious clear color[%d]: %f (window %d)\n", i, w->clear[i], id);
        }
    }

    // Validate time data
    CANVAS_ASSERT_VALID_DOUBLE(w->time.current);
    CANVAS_ASSERT_VALID_DOUBLE(w->time.delta);
    CANVAS_ASSERT_VALID_DOUBLE(w->time.fps);

    // Validate matrices (check for NaN)
    for (int i = 0; i < 16; i++)
    {
        CANVAS_ASSERT_VALID_FLOAT(w->view_matrix.m[i]);
        CANVAS_ASSERT_VALID_FLOAT(w->projection_matrix.m[i]);
    }

    // Validate title is null-terminated within bounds
    bool found_null = false;
    for (int i = 0; i < MAX_CANVAS_TITLE; i++)
    {
        if (w->title[i] == '\0')
        {
            found_null = true;
            break;
        }
    }
    if (!found_null)
    {
        CANVAS_ERR("Window %d title not null-terminated!\n", id);
        assert(0 && "title buffer overflow");
    }

    // Validate enums
    CANVAS_ASSERT_RANGE(w->cursor, CANVAS_CURSOR_HIDDEN, CANVAS_CURSOR_WAIT);
    CANVAS_ASSERT_RANGE(w->active_cursor, CANVAS_CURSOR_HIDDEN, CANVAS_CURSOR_WAIT);

    // Sanity check dimensions
    if (w->width < 0 || w->width > 32768)
    {
        CANVAS_ERR("Suspicious window width: %" PRId64 " (window %d)\n", w->width, id);
    }
    if (w->height < 0 || w->height > 32768)
    {
        CANVAS_ERR("Suspicious window height: %" PRId64 " (window %d)\n", w->height, id);
    }
}

static void _canvas_paranoid_validate_all(void)
{
    CANVAS_DBG("=== PARANOID VALIDATION START (op #%llu) ===\n",
               (unsigned long long)_canvas_op_sequence);

    // Check all canaries first
    _canvas_check_canaries();

    // Validate context isn't corrupted
    CANVAS_ASSERT_NOT_POISONED(&canvas_info, sizeof(canvas_info));

    // Run standard validations
    _canvas_validate_all_windows();
    _canvas_validate_all_displays();
    _canvas_validate_all_pointers();

    // Deep validate each window
    for (int i = 0; i < MAX_CANVAS; i++)
    {
        _canvas_validate_window_sanity(i);
    }

    // Deep validate each pointer
    for (int i = 0; i < CANVAS_POINTER_BUDGET; i++)
    {
        _canvas_validate_pointer_sanity(&canvas_info.pointers[i]);
    }

    // Validate time data
    CANVAS_ASSERT_VALID_DOUBLE(canvas_info.time.current);
    CANVAS_ASSERT_VALID_DOUBLE(canvas_info.time.delta);
    CANVAS_ASSERT_VALID_DOUBLE(canvas_info.time.fps);

    // Update tracking
    canvas_info._last_validated_op = _canvas_op_sequence;

    CANVAS_DBG("=== PARANOID VALIDATION COMPLETE ===\n");
}

// Call this at critical points to ensure everything is valid
#define CANVAS_PARANOID_CHECK()                          \
    do                                                   \
    {                                                    \
        _canvas_check_canaries();                        \
        CANVAS_ASSERT_NOT_POISONED(&canvas_info,         \
                                   sizeof(canvas_info)); \
    } while (0)

#else // CANVAS_VALIDATION < 5

#define CANVAS_PARANOID_CHECK() ((void)0)
static inline void _canvas_init_canaries(void) {}

#endif // CANVAS_VALIDATION >= 5

#endif

canvas_pointer *canvas_get_primary_pointer(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);
    CANVAS_ASSERT_RANGE(canvas_info.pointer_count, 0, CANVAS_POINTER_BUDGET);

    if (canvas_info.pointer_count == 0)
        canvas_info.pointer_count = 1;

    canvas_pointer *p = &canvas_info.pointers[0];
    CANVAS_ASSERT_NOT_NULL(p);

    if (!p->active)
    {
#if CANVAS_VALIDATION >= 5
        uint64_t saved_head = p->_canary_head;
        uint64_t saved_tail = p->_canary_tail;
#endif
        memset(p, 0, sizeof(canvas_pointer));
#if CANVAS_VALIDATION >= 5
        p->_canary_head = saved_head;
        p->_canary_tail = saved_tail;
#endif
        p->id = 0;
        p->type = CANVAS_POINTER_MOUSE;
        p->window_id = window_id;
        p->active = true;
        p->_sample_index = 0;
        CANVAS_DBG("Created primary pointer for window %d\n", window_id);
    }

    CANVAS_ASSERT(p->active);
    CANVAS_ASSERT_RANGE(p->id, 0, CANVAS_POINTER_BUDGET - 1);
    CANVAS_ASSERT_RANGE(p->_sample_index, 0, CANVAS_POINTER_SAMPLE_FRAMES - 1);
    CANVAS_RETURN(p);
}

#define C_RTLD_NOW 0x00002
#define C_RTLD_LOCAL 0x00001

#define CANVAS_BOUNDS(window_id)                           \
    do                                                     \
    {                                                      \
        CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1); \
        if (window_id < 0 || window_id >= MAX_CANVAS)      \
        {                                                  \
            CANVAS_ERR("bogus window: %d\n", window_id);   \
            return CANVAS_INVALID;                         \
        }                                                  \
    } while (0)

#define CANVAS_DISPLAY_BOUNDS(display_id)                     \
    do                                                        \
    {                                                         \
        CANVAS_ASSERT_RANGE(display_id, 0, MAX_DISPLAYS - 1); \
        if (display_id < 0 || display_id >= MAX_DISPLAYS)     \
        {                                                     \
            CANVAS_ERR("bogus display: %d\n", display_id);    \
            return CANVAS_INVALID;                            \
        }                                                     \
    } while (0)

#define CANVAS_VALID(window_id)                                                      \
    CANVAS_BOUNDS(window_id);                                                        \
    do                                                                               \
    {                                                                                \
        CANVAS_ASSERT_MSG(canvas_info.canvas[window_id]._valid, "window not valid"); \
        if (!canvas_info.canvas[window_id]._valid)                                   \
        {                                                                            \
            CANVAS_ERR("window %d is not valid\n", window_id);                       \
            return CANVAS_INVALID;                                                   \
        }                                                                            \
    } while (0)

#define CANVAS_VALID_PTR(window_id)                                                          \
    do                                                                                       \
    {                                                                                        \
        CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);                                   \
        if (window_id < 0 || window_id >= MAX_CANVAS)                                        \
            return NULL;                                                                     \
        CANVAS_ASSERT_MSG(canvas_info.canvas[window_id]._valid, "window not valid for ptr"); \
        if (!canvas_info.canvas[window_id]._valid)                                           \
            return NULL;                                                                     \
    } while (0)

int _canvas_get_free()
{
    CANVAS_ENTER_FUNC();
    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (!canvas_info.canvas[i]._valid)
        {
            CANVAS_DBG("Found free window slot: %d\n", i);
            CANVAS_RETURN(i);
        }
    }

    CANVAS_WARN("no free window slots\n");
    CANVAS_RETURN(CANVAS_ERR_NO_FREE);
}

int _canvas_window_index(void *window)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(window);

    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (canvas_info.canvas[i]._valid && canvas_info.canvas[i].window == window)
        {
            CANVAS_RETURN(i);
        }
    }

    CANVAS_WARN("bogus get_window_index: %p\n", window);
    CANVAS_RETURN(CANVAS_INVALID);
}

void *canvas_library_load(const char **names, int count)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(names);
    CANVAS_ASSERT_POSITIVE(count);

    for (int i = 0; i < count; ++i)
    {
        CANVAS_ASSERT_NOT_NULL(names[i]);
        CANVAS_DBG("Trying to load library: %s\n", names[i]);
#if defined(_WIN32)
        HMODULE h = LoadLibraryA(names[i]);
#else
        void *h = dlopen(names[i], RTLD_NOW | RTLD_LOCAL);
#endif
        if (h)
        {
            CANVAS_INFO("Loaded library: %s\n", names[i]);
            CANVAS_RETURN(h);
        }
    }
    CANVAS_WARN("Failed to load any library variant\n");
    CANVAS_RETURN(NULL);
}

void *canvas_library_symbol(void *lib, const char *sym)
{
    CANVAS_ASSERT_NOT_NULL(lib);
    CANVAS_ASSERT_NOT_NULL(sym);

#if defined(_WIN32)
    void *result = (void *)GetProcAddress((HMODULE)lib, sym);
#else
    void *result = dlsym(lib, sym);
#endif

    if (!result)
    {
        CANVAS_WARN("Failed to load symbol: %s\n", sym);
    }
    return result;
}

void canvas_library_close(void *lib)
{
    CANVAS_ASSERT_NOT_NULL(lib);
#if defined(_WIN32)
    FreeLibrary((HMODULE)lib);
#else
    dlclose(lib);
#endif
}

#if defined(__APPLE__)

#include <TargetConditionals.h>
#include <objc/objc.h>
#include <objc/message.h>
#include <time.h>
#include <mach/mach_time.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>
#include <IOKit/hid/IOHIDManager.h>
#include <CoreFoundation/CoreFoundation.h>

typedef void *objc_id;
typedef void *objc_sel;

typedef enum
{
    NSWindowStyleMaskTitled = 1 << 0,
    NSWindowStyleMaskClosable = 1 << 1,
    NSWindowStyleMaskMiniaturizable = 1 << 2,
    NSWindowStyleMaskResizable = 1 << 3,
    NSWindowStyleMaskFullSizeContent = 1 << 15,
} NSWindowStyleMask;

typedef struct
{
    double r, g, b, a;
} _MTLClearColor;

typedef struct
{
    double x, y, w, h;
} _CGRect;

typedef struct
{
    objc_id view;
    objc_id layer;
    double scale;
    int saved_x, saved_y, saved_width, saved_height;
    unsigned long saved_style_mask;
} canvas_data;

typedef struct
{
    objc_id pool;
    objc_id app;
    mach_timebase_info_data_t timebase;

    IOHIDManagerRef hid_manager;

    objc_id device;
    objc_id queue;
} canvas_platform_macos;

extern objc_id MTLCreateSystemDefaultDevice(void);

typedef objc_id (*msg_send_id_id)(objc_id, objc_sel);
typedef void (*msg_send_void_id)(objc_id, objc_sel);
typedef void (*msg_send_void_id_id)(objc_id, objc_sel, objc_id);
typedef void (*msg_send_void_id_bool)(objc_id, objc_sel, int);
typedef void (*msg_send_void_id_long)(objc_id, objc_sel, long);
typedef double (*msg_send_dbl_id)(objc_id, objc_sel);
typedef unsigned long (*msg_send_ulong_id)(objc_id, objc_sel);

static inline objc_sel sel_c(const char *s) { return sel_registerName(s); }
static inline objc_id cls(const char *s) { return objc_getClass(s); }
typedef objc_id (*MSG_id_id)(objc_id, objc_sel);
typedef objc_id (*MSG_id_id_id)(objc_id, objc_sel, objc_id);
typedef objc_id (*MSG_id_id_rect)(objc_id, objc_sel, _CGRect);
typedef objc_id (*MSG_id_id_ulong)(objc_id, objc_sel, unsigned long);
typedef objc_id (*MSG_id_id_charp)(objc_id, objc_sel, const char *);
typedef double (*MSG_dbl_id)(objc_id, objc_sel);
typedef unsigned long (*MSG_ulong_id)(objc_id, objc_sel);
typedef void (*MSG_void_id)(objc_id, objc_sel);
typedef void (*MSG_void_id_id)(objc_id, objc_sel, objc_id);
typedef void (*MSG_void_id_bool)(objc_id, objc_sel, int);
typedef void (*MSG_void_id_long)(objc_id, objc_sel, long);
typedef void (*MSG_void_id_ulong)(objc_id, objc_sel, unsigned long);
typedef void (*MSG_void_id_clear)(objc_id, objc_sel, _MTLClearColor);
typedef void (*MSG_void_id_rect)(objc_id, objc_sel, _CGRect);
typedef _CGRect (*MSG_rect_id)(objc_id, objc_sel);

static inline objc_id msg_id(objc_id obj, const char *sel)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    return ((msg_send_id_id)objc_msgSend)(obj, sel_c(sel));
}

static inline void msg_void(objc_id obj, const char *sel)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    ((msg_send_void_id)objc_msgSend)(obj, sel_c(sel));
}

static inline void msg_void_id(objc_id obj, const char *sel, objc_id arg)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    ((msg_send_void_id_id)objc_msgSend)(obj, sel_c(sel), arg);
}

static inline void msg_void_bool(objc_id obj, const char *sel, bool val)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    ((msg_send_void_id_bool)objc_msgSend)(obj, sel_c(sel), val ? 1 : 0);
}

static inline void msg_void_long(objc_id obj, const char *sel, long val)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    ((msg_send_void_id_long)objc_msgSend)(obj, sel_c(sel), val);
}

static inline double msg_dbl(objc_id obj, const char *sel)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    return ((msg_send_dbl_id)objc_msgSend)(obj, sel_c(sel));
}

static inline unsigned long msg_ulong(objc_id obj, const char *sel)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    return ((msg_send_ulong_id)objc_msgSend)(obj, sel_c(sel));
}

static inline bool msg_bool(objc_id obj, const char *sel)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    typedef bool (*msg_send_bool)(objc_id, objc_sel);
    return ((msg_send_bool)objc_msgSend)(obj, sel_c(sel));
}

static inline void msg_void_rect_bool(objc_id obj, const char *sel, _CGRect rect, bool display)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    typedef void (*msg_send_rect_bool)(objc_id, objc_sel, _CGRect, int);
    ((msg_send_rect_bool)objc_msgSend)(obj, sel_c(sel), rect, display ? 1 : 0);
}

static inline objc_id nsstring_from_cstr(const char *str)
{
    if (!str || !str[0])
        return NULL;
    typedef objc_id (*msg_from_cstr)(objc_id, objc_sel, const char *);
    return ((msg_from_cstr)objc_msgSend)(cls("NSString"), sel_c("stringWithUTF8String:"), str);
}

static inline objc_id msg_id_id(objc_id obj, const char *sel, objc_id arg)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    typedef objc_id (*msg_send_id_id)(objc_id, objc_sel, objc_id);
    return ((msg_send_id_id)objc_msgSend)(obj, sel_c(sel), arg);
}

#if defined(__x86_64__)
typedef void (*msg_send_stret)(void *, objc_id, objc_sel);
static inline _CGRect msg_rect(objc_id obj, const char *sel)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    _CGRect rect;
    ((msg_send_stret)objc_msgSend_stret)(&rect, obj, sel_c(sel));
    return rect;
}
#else
static inline _CGRect msg_rect(objc_id obj, const char *sel)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    typedef _CGRect (*msg_send_rect)(objc_id, objc_sel);
    return ((msg_send_rect)objc_msgSend)(obj, sel_c(sel));
}
#endif

static inline void msg_void_size(objc_id obj, const char *sel, double width, double height)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    CANVAS_ASSERT(width > 0);
    CANVAS_ASSERT(height > 0);
    struct
    {
        double width;
        double height;
    } sz = {width, height};
    typedef void (*msg_send_size)(objc_id, objc_sel, typeof(sz));
    ((msg_send_size)objc_msgSend)(obj, sel_c(sel), sz);
}

static inline objc_id msg_id_rect(objc_id obj, const char *sel, _CGRect rect)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    typedef objc_id (*msg_send_rect)(objc_id, objc_sel, _CGRect);
    return ((msg_send_rect)objc_msgSend)(obj, sel_c(sel), rect);
}

static inline void msg_void_double(objc_id obj, const char *sel, double val)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    typedef void (*msg_send_dbl)(objc_id, objc_sel, double);
    ((msg_send_dbl)objc_msgSend)(obj, sel_c(sel), val);
}

static inline objc_id msg_id_ulong(objc_id obj, const char *sel, unsigned long val)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    typedef objc_id (*msg_send_ulong)(objc_id, objc_sel, unsigned long);
    return ((msg_send_ulong)objc_msgSend)(obj, sel_c(sel), val);
}

static inline void msg_void_clear_color(objc_id obj, const char *sel, _MTLClearColor color)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    typedef void (*msg_send_clear)(objc_id, objc_sel, _MTLClearColor);
    ((msg_send_clear)objc_msgSend)(obj, sel_c(sel), color);
}

static inline objc_id msg_id_id_descriptor(objc_id obj, const char *sel, objc_id descriptor)
{
    CANVAS_ASSERT_NOT_NULL(obj);
    CANVAS_ASSERT_NOT_NULL(sel);
    typedef objc_id (*msg_send_desc)(objc_id, objc_sel, objc_id);
    return ((msg_send_desc)objc_msgSend)(obj, sel_c(sel), descriptor);
}

static inline objc_id canvas_next_event(objc_id app, unsigned long long mask, objc_id until_date, objc_id mode, bool dequeue)
{
    CANVAS_ASSERT_NOT_NULL(app);
    typedef objc_id (*msg_send_event)(objc_id, objc_sel, unsigned long long, objc_id, objc_id, signed char);
    msg_send_event next = (msg_send_event)objc_msgSend;
    return next(app, sel_c("nextEventMatchingMask:untilDate:inMode:dequeue:"),
                mask, until_date, mode, dequeue ? 1 : 0);
}

typedef enum
{
    NSBackingStoreBuffered = 2,
} NSBackingStoreType;

typedef enum
{
    NSWindowTitleHidden = 1,
} NSWindowTitleVisibility;

typedef enum
{
    NSApplicationActivationPolicyRegular = 0,
} NSApplicationActivationPolicy;

typedef enum
{
    MTLPixelFormatBGRA8Unorm = 80,
} MTLPixelFormat;

typedef enum
{
    MTLLoadActionClear = 2,
} MTLLoadAction;

typedef enum
{
    MTLStoreActionStore = 1,
} MTLStoreAction;

static inline _CGRect make_rect(double x, double y, double w, double h)
{
    _CGRect r = {x, y, w, h};
    return r;
}

static inline _MTLClearColor make_clear_color(float r, float g, float b, float a)
{
    _MTLClearColor c = {r, g, b, a};
    return c;
}

static const char *_cursor_selector_names[] = {
    [CANVAS_CURSOR_HIDDEN] = NULL,
    [CANVAS_CURSOR_ARROW] = "arrowCursor",
    [CANVAS_CURSOR_TEXT] = "IBeamCursor",
    [CANVAS_CURSOR_CROSSHAIR] = "crosshairCursor",
    [CANVAS_CURSOR_HAND] = "pointingHandCursor",
    [CANVAS_CURSOR_SIZE_NS] = "resizeUpDownCursor",
    [CANVAS_CURSOR_SIZE_EW] = "resizeLeftRightCursor",
    [CANVAS_CURSOR_SIZE_NESW] = "closedHandCursor",
    [CANVAS_CURSOR_SIZE_NWSE] = "closedHandCursor",
    [CANVAS_CURSOR_SIZE_ALL] = "closedHandCursor",
    [CANVAS_CURSOR_NOT_ALLOWED] = "operationNotAllowedCursor",
    [CANVAS_CURSOR_WAIT] = "arrowCursor",
};

canvas_data _canvas_data[MAX_CANVAS];
canvas_platform_macos canvas_macos;

#endif

#if defined(_WIN32)

#include <windows.h>
#include <dwmapi.h>

#define INITGUID

#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "winmm.lib")

static LARGE_INTEGER _canvas_qpc_frequency = {0};
static LARGE_INTEGER _canvas_start_counter = {0};

typedef struct
{
    HINSTANCE instance;
    ATOM class;

    ID3D12Device *device;
    ID3D12CommandQueue *cmdQueue;
    IDXGIFactory4 *factory;
    ID3D12CommandAllocator *cmdAllocator;
    ID3D12GraphicsCommandList *cmdList;
    ID3D12DescriptorHeap *rtvHeap;
    ID3D12Fence *fence;
    UINT64 fence_value;
    HANDLE fence_event;
    UINT rtvDescriptorSize;

    HMONITOR monitors[MAX_DISPLAYS];
} _canvas_platform_windows;

typedef struct
{
    IDXGISwapChain3 *swapChain;
    ID3D12Resource *backBuffers[2];
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
    int saved_x, saved_y, saved_width, saved_height;
    HWND saved_z_order;
    DWORD saved_style;
} canvas_data;

canvas_data _canvas_data[MAX_CANVAS];
_canvas_platform_windows canvas_win32;

#endif

#if defined(__linux__)

#define CANVAS_VULKAN

#include <time.h>

struct wl_display;
struct wl_registry;
struct wl_compositor;
struct wl_surface;
struct wl_interface;
struct wl_proxy;

typedef struct _XDisplay Display;
typedef unsigned long Window;
typedef unsigned long Atom;
typedef unsigned long XID;

typedef struct
{
    int type;
    char pad[256 - sizeof(int)];
} XEvent;

static struct
{
    canvas_library_handle library;

    struct wl_display *(*wl_display_connect)(const char *);
    void (*wl_display_disconnect)(struct wl_display *);
    int (*wl_display_dispatch)(struct wl_display *);
    int (*wl_display_roundtrip)(struct wl_display *);
    void *(*wl_display_get_registry)(struct wl_display *);
    void *(*wl_registry_bind)(struct wl_registry *, uint32_t, const struct wl_interface *, uint32_t);
    int (*wl_registry_add_listener)(struct wl_registry *, const void *, void *);
    void (*wl_registry_destroy)(struct wl_registry *);
    struct wl_surface *(*wl_compositor_create_surface)(struct wl_compositor *);
    void (*wl_surface_destroy)(struct wl_surface *);
    void (*wl_compositor_destroy)(struct wl_compositor *);
    void (*wl_proxy_destroy)(struct wl_proxy *);

    const struct wl_interface *wl_compositor_interface;
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_shm *shm;
    struct xdg_wm_base *xdg_wm_base;
} wl;

typedef struct
{
    int type;
    Display *display;
    unsigned long serial;
    unsigned char error_code;
    unsigned char request_code;
    unsigned char minor_code;
    XID resourceid;
} XErrorEvent;

static struct
{
    canvas_library_handle library;

    bool cursors_loaded;
    unsigned long cursors[11];

    Display *(*XOpenDisplay)(const char *);
    void (*XSetErrorHandler)(int (*)(Display *, XErrorEvent *));
    void (*XGetErrorText)(Display *, int, char *, int);

    int (*XCloseDisplay)(Display *);
    Window (*XCreateSimpleWindow)(Display *, Window, int, int, unsigned int, unsigned int, unsigned int, unsigned long, unsigned long);
    int (*XDestroyWindow)(Display *, Window);
    int (*XMapWindow)(Display *, Window);
    int (*XStoreName)(Display *, Window, const char *);
    int (*XSelectInput)(Display *, Window, long);
    int (*XNextEvent)(Display *, XEvent *);
    int (*XSendEvent)(Display *, Window, bool, long, XEvent *);
    int (*XPending)(Display *);
    Window (*XRootWindow)(Display *, int);
    int (*XFlush)(Display *);
    int (*XMoveResizeWindow)(Display *, Window, int, int, unsigned int, unsigned int);
    int (*XDisplayWidth)(Display *, int screen_number);
    int (*XDisplayHeight)(Display *, int screen_number);
    Atom (*XInternAtom)(Display *, const char *, int);
    int (*XIconifyWindow)(Display *, Window, int);
    int (*XSetWMProtocols)(Display *, Window, Atom *, int);
    unsigned long (*XBlackPixel)(Display *, int);
    unsigned long (*XWhitePixel)(Display *, int);
    Window (*XDefaultRootWindow)(Display *);
    int (*XDefaultScreen)(Display *);
    int (*XChangeProperty)(Display *, Window, Atom, Atom, int, int, const unsigned char *, int);
    int (*XGetWindowAttributes)(Display *, Window, void *);
    int (*XGrabPointer)(Display *, Window, bool, unsigned int, int, int, Window, int, unsigned long);
    int (*XUngrabPointer)(Display *, unsigned long);
    int (*XMoveWindow)(Display *, Window, int, int);
    void *(*XCreateFontCursor)(Display *, unsigned int);
    int (*XDefineCursor)(Display *, Window, unsigned long);
    int (*XFree)(void *);
    bool (*XQueryPointer)(Display *, Window, Window *, Window *, int *, int *, int *, int *, unsigned int *);
    bool (*XTranslateCoordinates)(Display *, Window, Window, int, int, int *, int *, Window *);
    int (*XFreeCursor)(Display *, unsigned long);

    int (*XGetWindowProperty)(Display *, Window, Atom, long, long, bool, Atom, Atom *, int *, unsigned long *, unsigned long *, unsigned char **);

    Atom internal_atom;
    Display *display;
} x11;

typedef struct
{
    void *timestamp;
    void *configTimestamp;
    int ncrtc;
    unsigned long *crtcs;
    int noutput;
    unsigned long *outputs;
    int nmode;
    void *modes;
} XRRScreenResources;

typedef struct
{
    long timestamp;
    int x, y;
    unsigned int width, height;
    unsigned long mode;
    unsigned short rotation;
    int noutput;
    unsigned long *outputs;
    unsigned short rotations;
    int npossible;
    unsigned long *possible;
} XRRCrtcInfo;

typedef struct
{
    long timestamp;
    unsigned long crtc;
    char *name;
    int nameLen;
    unsigned long mm_width;
    unsigned long mm_height;
    unsigned short connection;
    unsigned short subpixel_order;
    int ncrtc;
    unsigned long *crtcs;
    int nclone;
    unsigned long *clones;
    int nmode;
    unsigned long *modes;
    int npreferred;
} XRROutputInfo;

typedef struct
{
    unsigned long id;
    unsigned short width;
    unsigned short height;
    unsigned long dotClock;
    unsigned short hSyncStart;
    unsigned short hSyncEnd;
    unsigned short hTotal;
    unsigned short hSkew;
    unsigned short vSyncStart;
    unsigned short vSyncEnd;
    unsigned short vTotal;
    char *name;
    unsigned int nameLength;
    unsigned long modeFlags;
} XRRModeInfo;

typedef struct
{
    int type;
    unsigned long serial;
    bool send_event;
    Display *display;
    Window window;
    Window root;
    Window subwindow;
    unsigned long time;
    int x, y;
    int x_root, y_root;
    unsigned int state;
    unsigned int keycode;
    bool same_screen;
} XKeyEvent;
typedef XKeyEvent XKeyPressedEvent;
typedef XKeyEvent XKeyReleasedEvent;

static struct
{
    canvas_library_handle library;

    void *(*XRRGetScreenResourcesCurrent)(Display *, Window);
    void (*XRRFreeScreenResources)(void *);
    void *(*XRRGetCrtcInfo)(Display *, void *, unsigned long);
    void (*XRRFreeCrtcInfo)(void *);
    void *(*XRRGetOutputInfo)(Display *, void *, unsigned long);
    void (*XRRFreeOutputInfo)(void *);
    unsigned long (*XRRGetOutputPrimary)(Display *, Window);
} xrandr;

#define None 0L
#define ButtonPressMask (1L << 2)
#define ButtonReleaseMask (1L << 3)
#define PointerMotionMask (1L << 6)

#define XA_CARDINAL ((Atom)6)
#define PropModeReplace 0

#define LOAD_X11(name)                                  \
    x11.name = dlsym(x11.library, #name);               \
    if (!x11.name)                                      \
    {                                                   \
        CANVAS_ERR("loading " #name ": %s", dlerror()); \
        dlclose(x11.library);                           \
        return CANVAS_ERR_LOAD_SYMBOL;                  \
    }

#define LOAD_WL(name)                                   \
    wl.name = dlsym(wl.library, #name);                 \
    if (!wl.name)                                       \
    {                                                   \
        CANVAS_ERR("loading " #name ": %s", dlerror()); \
        dlclose(wl.library);                            \
        return CANVAS_ERR_LOAD_SYMBOL;                  \
    }

const char *canvas_wayland_library_names[2] = {"libwayland-client.so.0", "libwayland-client.so"};
const char *canvas_x11_library_names[2] = {"libX11.so.6", "libX11.so"};
const char *canvas_xrandr_library_names[2] = {"libXrandr.so.2", "libXrandr.so"};

typedef struct
{
    int type;
    unsigned long serial;
    int send_event;
    Display *display;
    Window window;
} _x11_event;

typedef struct
{
    int type;
    unsigned long serial;
    int send_event;
    Display *display;
    Window window;
    Atom message_type;
    int format;
    union
    {
        char b[20];
        short s[10];
        long l[5];
    } data;
} XClientMessageEvent;

typedef struct
{
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long input_mode;
    unsigned long status;
} MotifWmHints;

typedef struct
{
    int type;
    unsigned long serial;
    int send_event;
    Display *display;
    Window window;
    Window root;
    Window subwindow;
    unsigned long time;
    int x, y;
    int x_root, y_root;
    unsigned int state;
    char is_hint;
    int same_screen;
} XMotionEvent;

typedef struct
{
    int type;
    unsigned long serial;
    int send_event;
    Display *display;
    Window window;
    Window root;
    Window subwindow;
    unsigned long time;
    int x, y;
    int x_root, y_root;
    unsigned int state;
    unsigned int button;
    int same_screen;
} XButtonEvent;

typedef struct
{
    bool x11_atoms_initialized;
    Atom x11_wm_delete_window;
    Atom x11_wm_protocols;
    Atom x11_wm_state;
    Atom x11_net_wm_state;
    Atom x11_net_wm_state_maximized_horz;
    Atom x11_net_wm_state_maximized_vert;
    Atom x11_net_wm_state_fullscreen;
    Atom x11_net_wm_moveresize;
    Atom x11_motif_wm_hints;

    int last_button_press_time;
    int last_button_press_x;
    int last_button_press_y;

    bool client_set;
    int saved_x, saved_y, saved_width, saved_height;
} canvas_data;

typedef struct
{
    int x, y;
    int width, height;
    int border_width;
    int depth;
    void *visual;
    Window root;
    int class;
    int bit_gravity;
    int win_gravity;
    int backing_store;
    unsigned long backing_planes;
    unsigned long backing_pixel;
    bool save_under;
    int colormap;
    bool map_installed;
    int map_state;
    long all_event_masks;
    long your_event_mask;
    long do_not_propagate_mask;
    bool override_redirect;
    void *screen;
} XWindowAttributes;

typedef struct
{
    int type;
    unsigned long serial;
    bool send_event;
    Display *display;
    Window event;
    Window window;
    int x, y;
    int width, height;
    int border_width;
    Window above;
    bool override_redirect;
} XConfigureEvent;

typedef struct
{
    int type;
    unsigned long serial;
    bool send_event;
    Display *display;
    Window window;
    Atom atom;
    unsigned long time;
    int state;
} XPropertyEvent;

#define X11_KeyPress 2
#define X11_KeyRelease 3
#define X11_ButtonPress 4
#define X11_ButtonRelease 5
#define X11_MotionNotify 6
#define X11_EnterNotify 7
#define X11_LeaveNotify 8
#define X11_FocusIn 9
#define X11_FocusOut 10
#define X11_Expose 12
#define X11_ConfigureNotify 22
#define X11_ClientMessage 33
#define X11_MapNotify 19
#define X11_UnmapNotify 18
#define X11_GrabModeAsync 1
#define X11_CurrentTime 0L
#define X11_PropertyNotify 28

#define RESIZE_EDGE_NONE 0
#define RESIZE_EDGE_TOP 1
#define RESIZE_EDGE_BOTTOM 2
#define RESIZE_EDGE_LEFT 4
#define RESIZE_EDGE_RIGHT 8
#define RESIZE_BORDER 8

#define _NET_WM_MOVERESIZE_SIZE_TOPLEFT 0
#define _NET_WM_MOVERESIZE_SIZE_TOP 1
#define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT 2
#define _NET_WM_MOVERESIZE_SIZE_RIGHT 3
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT 4
#define _NET_WM_MOVERESIZE_SIZE_BOTTOM 5
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT 6
#define _NET_WM_MOVERESIZE_SIZE_LEFT 7
#define _NET_WM_MOVERESIZE_MOVE 8

#define XA_ATOM ((Atom)4)
#define MWM_HINTS_DECORATIONS (1L << 1)
#define MWM_DECOR_BORDER (1L << 1)
#define MWM_DECOR_RESIZEH (1L << 2)

bool _canvas_x11_flush = false;
bool _canvas_using_wayland = false;

canvas_data _canvas_data[MAX_CANVAS];

#endif

#ifdef CANVAS_VULKAN

#include <vulkan/vulkan.h>

#ifdef __linux__
typedef unsigned long VisualID;
#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan_xlib.h>

#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan_wayland.h>
#endif

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_win32.h>
#endif

#ifdef __APPLE__
#define VK_USE_PLATFORM_METAL_EXT
#endif

#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_SWAPCHAIN_IMAGES 3

#if defined(_WIN32)
#define canvas_vulkan_names 1
#define canvas_vulkan_library_names {"vulkan-1.dll"}
#elif defined(__linux__)
#define canvas_vulkan_names 2
#define canvas_vulkan_library_names {"libvulkan.so.1", "libvulkan.so"}
#elif defined(__APPLE__)
#define canvas_vulkan_names 6
#define canvas_vulkan_library_names {"libvulkan.dylib", "libvulkan.1.dylib", "libMoltenVK.dylib", "vulkan.framework/vulkan", "MoltenVK.framework/MoltenVK", "/usr/local/lib/libvulkan.dylib"}
#endif

#define VK_LOAD_INSTANCE_FUNC(name)                                                    \
    vk_info.name = (PFN_##name)vk_info.vkGetInstanceProcAddr(vk_info.instance, #name); \
    if (!vk_info.name)                                                                 \
    {                                                                                  \
        CANVAS_ERR("failed to load instance function: " #name "\n");                   \
        return CANVAS_ERR_LOAD_SYMBOL;                                                 \
    }

#define VK_LOAD_DEVICE_FUNC(name)                                                  \
    vk_info.name = (PFN_##name)vk_info.vkGetDeviceProcAddr(vk_info.device, #name); \
    if (!vk_info.name)                                                             \
    {                                                                              \
        CANVAS_ERR("failed to load device function: " #name "\n");                 \
        return CANVAS_ERR_LOAD_SYMBOL;                                             \
    }

#define VK_CHECK(result, msg)                                 \
    do                                                        \
    {                                                         \
        CANVAS_ASSERT_MSG((result) == VK_SUCCESS, msg);       \
        if ((result) != VK_SUCCESS)                           \
        {                                                     \
            CANVAS_ERR("%s (VkResult: %d)\n", msg, (result)); \
            return CANVAS_FAIL;                               \
        }                                                     \
    } while (0)

static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data)
{
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        CANVAS_WARN("vulkan validation: %s\n", callback_data->pMessage);

    return VK_FALSE;
}

static struct
{
    canvas_library_handle library;

    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    int graphics_family;
    int present_family;
    bool validation_enabled;
    VkDebugUtilsMessengerEXT debug_messenger;

    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
    PFN_vkCreateInstance vkCreateInstance;
    PFN_vkDestroyInstance vkDestroyInstance;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
    PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
    PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
    PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
    PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;

    PFN_vkCreateDevice vkCreateDevice;
    PFN_vkDestroyDevice vkDestroyDevice;
    PFN_vkGetDeviceQueue vkGetDeviceQueue;
    PFN_vkDeviceWaitIdle vkDeviceWaitIdle;

    PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;

#ifdef __linux__
    PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;
    PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;
#endif
#ifdef _WIN32
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
#endif
#ifdef __APPLE__
    PFN_vkCreateMetalSurfaceEXT vkCreateMetalSurfaceEXT;
#endif

    PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
    PFN_vkQueuePresentKHR vkQueuePresentKHR;

    PFN_vkCreateCommandPool vkCreateCommandPool;
    PFN_vkDestroyCommandPool vkDestroyCommandPool;
    PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
    PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
    PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
    PFN_vkEndCommandBuffer vkEndCommandBuffer;
    PFN_vkResetCommandBuffer vkResetCommandBuffer;

    PFN_vkCreateSemaphore vkCreateSemaphore;
    PFN_vkDestroySemaphore vkDestroySemaphore;
    PFN_vkCreateFence vkCreateFence;
    PFN_vkDestroyFence vkDestroyFence;
    PFN_vkWaitForFences vkWaitForFences;
    PFN_vkResetFences vkResetFences;

    PFN_vkQueueSubmit vkQueueSubmit;
    PFN_vkQueueWaitIdle vkQueueWaitIdle;

    PFN_vkCreateRenderPass vkCreateRenderPass;
    PFN_vkDestroyRenderPass vkDestroyRenderPass;
    PFN_vkCreateFramebuffer vkCreateFramebuffer;
    PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
    PFN_vkCreateImageView vkCreateImageView;
    PFN_vkDestroyImageView vkDestroyImageView;

    PFN_vkCreateBuffer vkCreateBuffer;
    PFN_vkDestroyBuffer vkDestroyBuffer;
    PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
    PFN_vkBindBufferMemory vkBindBufferMemory;
    PFN_vkCmdCopyBuffer vkCmdCopyBuffer;

    PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
    PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
    PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;

    PFN_vkCreateImage vkCreateImage;
    PFN_vkDestroyImage vkDestroyImage;
    PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;

    PFN_vkAllocateMemory vkAllocateMemory;
    PFN_vkFreeMemory vkFreeMemory;
    PFN_vkBindImageMemory vkBindImageMemory;
    PFN_vkMapMemory vkMapMemory;
    PFN_vkUnmapMemory vkUnmapMemory;

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;

    PFN_vkResetCommandPool vkResetCommandPool;

} vk_info = {0};

typedef struct
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    uint32_t format_count;
    VkPresentModeKHR *present_modes;
    uint32_t present_mode_count;
} SwapchainSupportDetails;

typedef struct
{
    VkFence submit_frame;
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    VkSemaphore acquire_semaphore;
    VkSemaphore present_semaphore;
} canvas_vk_per_frame;

typedef struct
{
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkImage swapchain_images[MAX_SWAPCHAIN_IMAGES];
    VkImageView swapchain_image_views[MAX_SWAPCHAIN_IMAGES];
    VkFramebuffer framebuffers[MAX_SWAPCHAIN_IMAGES];
    uint32_t swapchain_image_count;
    VkFormat swapchain_format;
    VkExtent2D swapchain_extent;

    VkCommandPool command_pool;
    VkCommandBuffer command_buffers[MAX_SWAPCHAIN_IMAGES];

    VkSemaphore image_available_semaphores[MAX_SWAPCHAIN_IMAGES];
    VkSemaphore render_finished_semaphores[MAX_SWAPCHAIN_IMAGES];
    VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT];
    VkFence images_in_flight[MAX_SWAPCHAIN_IMAGES];
    uint32_t current_frame;

    VkImage depth_image;
    VkDeviceMemory depth_memory;
    VkImageView depth_view;

    VkRenderPass render_pass;
    bool needs_resize;
    bool recreating_swapchain;
    bool initialized;
} canvas_vulkan_window;

static canvas_vulkan_window vk_windows[MAX_CANVAS] = {0};
const char *vulkan_library_names[canvas_vulkan_names] = canvas_vulkan_library_names;

static bool vk_check_validation_layers()
{
    CANVAS_ENTER_FUNC();

    if (!vk_info.vkEnumerateInstanceLayerProperties)
    {
        CANVAS_WARN("vkEnumerateInstanceLayerProperties not available\n");
        CANVAS_RETURN(false);
    }

    uint32_t layer_count = 0;
    VkResult result = vk_info.vkEnumerateInstanceLayerProperties(&layer_count, NULL);

    if (result != VK_SUCCESS)
    {
        CANVAS_WARN("failed to enumerate validation layers (code: %d), disabling validation\n", result);
        CANVAS_RETURN(false);
    }

    if (layer_count == 0)
    {
        CANVAS_VERBOSE("no validation layers available\n");
        CANVAS_RETURN(false);
    }

    VkLayerProperties *available_layers = (VkLayerProperties *)malloc(layer_count * sizeof(VkLayerProperties));
    CANVAS_ASSERT_NOT_NULL(available_layers);
    if (!available_layers)
    {
        CANVAS_WARN("failed to allocate memory for layer properties\n");
        CANVAS_RETURN(false);
    }

    result = vk_info.vkEnumerateInstanceLayerProperties(&layer_count, available_layers);
    if (result != VK_SUCCESS)
    {
        CANVAS_WARN("failed to get layer properties (code: %d), disabling validation\n", result);
        free(available_layers);
        CANVAS_RETURN(false);
    }

    bool found = false;
    const char *validation_layer = "VK_LAYER_KHRONOS_validation";

    for (uint32_t i = 0; i < layer_count; i++)
    {
        if (strcmp(validation_layer, available_layers[i].layerName) == 0)
        {
            found = true;
            CANVAS_DBG("Found validation layer: %s\n", validation_layer);
            break;
        }
    }

    free(available_layers);

    if (!found)
        CANVAS_VERBOSE("VK_LAYER_KHRONOS_validation not found, validation disabled\n");

    CANVAS_RETURN(found);
}

static int vk_load_instance_functions()
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(vk_info.instance);
    CANVAS_ASSERT_NOT_NULL(vk_info.vkGetInstanceProcAddr);

    VK_LOAD_INSTANCE_FUNC(vkDestroyInstance);
    VK_LOAD_INSTANCE_FUNC(vkEnumeratePhysicalDevices);
    VK_LOAD_INSTANCE_FUNC(vkEnumerateDeviceExtensionProperties);
    VK_LOAD_INSTANCE_FUNC(vkGetPhysicalDeviceQueueFamilyProperties);
    VK_LOAD_INSTANCE_FUNC(vkGetPhysicalDeviceProperties);
    VK_LOAD_INSTANCE_FUNC(vkGetPhysicalDeviceFeatures);
    VK_LOAD_INSTANCE_FUNC(vkGetPhysicalDeviceMemoryProperties);
    VK_LOAD_INSTANCE_FUNC(vkCreateDevice);
    VK_LOAD_INSTANCE_FUNC(vkGetDeviceQueue);

    VK_LOAD_INSTANCE_FUNC(vkDestroySurfaceKHR);
    VK_LOAD_INSTANCE_FUNC(vkGetPhysicalDeviceSurfaceSupportKHR);
    VK_LOAD_INSTANCE_FUNC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    VK_LOAD_INSTANCE_FUNC(vkGetPhysicalDeviceSurfaceFormatsKHR);
    VK_LOAD_INSTANCE_FUNC(vkGetPhysicalDeviceSurfacePresentModesKHR);

#ifdef __linux__
    vk_info.vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)vk_info.vkGetInstanceProcAddr(vk_info.instance, "vkCreateXlibSurfaceKHR");
    vk_info.vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)vk_info.vkGetInstanceProcAddr(vk_info.instance, "vkCreateWaylandSurfaceKHR");
#endif
#ifdef _WIN32
    VK_LOAD_INSTANCE_FUNC(vkCreateWin32SurfaceKHR);
#endif
#ifdef __APPLE__
    VK_LOAD_INSTANCE_FUNC(vkCreateMetalSurfaceEXT);
#endif

    if (vk_info.validation_enabled)
    {
        vk_info.vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vk_info.vkGetInstanceProcAddr(vk_info.instance, "vkCreateDebugUtilsMessengerEXT");
        vk_info.vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vk_info.vkGetInstanceProcAddr(vk_info.instance, "vkDestroyDebugUtilsMessengerEXT");
    }

    CANVAS_RETURN(CANVAS_OK);
}

static int vk_load_device_functions()
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(vk_info.device);
    CANVAS_ASSERT_NOT_NULL(vk_info.vkGetDeviceProcAddr);

    VK_LOAD_DEVICE_FUNC(vkDestroyDevice);
    VK_LOAD_DEVICE_FUNC(vkDeviceWaitIdle);
    VK_LOAD_DEVICE_FUNC(vkCreateSwapchainKHR);
    VK_LOAD_DEVICE_FUNC(vkDestroySwapchainKHR);
    VK_LOAD_DEVICE_FUNC(vkGetSwapchainImagesKHR);
    VK_LOAD_DEVICE_FUNC(vkAcquireNextImageKHR);
    VK_LOAD_DEVICE_FUNC(vkQueuePresentKHR);
    VK_LOAD_DEVICE_FUNC(vkCreateCommandPool);
    VK_LOAD_DEVICE_FUNC(vkDestroyCommandPool);
    VK_LOAD_DEVICE_FUNC(vkAllocateCommandBuffers);
    VK_LOAD_DEVICE_FUNC(vkFreeCommandBuffers);
    VK_LOAD_DEVICE_FUNC(vkBeginCommandBuffer);
    VK_LOAD_DEVICE_FUNC(vkEndCommandBuffer);
    VK_LOAD_DEVICE_FUNC(vkResetCommandBuffer);
    VK_LOAD_DEVICE_FUNC(vkCreateSemaphore);
    VK_LOAD_DEVICE_FUNC(vkDestroySemaphore);
    VK_LOAD_DEVICE_FUNC(vkCreateFence);
    VK_LOAD_DEVICE_FUNC(vkDestroyFence);
    VK_LOAD_DEVICE_FUNC(vkWaitForFences);
    VK_LOAD_DEVICE_FUNC(vkResetFences);
    VK_LOAD_DEVICE_FUNC(vkQueueSubmit);
    VK_LOAD_DEVICE_FUNC(vkQueueWaitIdle);
    VK_LOAD_DEVICE_FUNC(vkCreateRenderPass);
    VK_LOAD_DEVICE_FUNC(vkDestroyRenderPass);
    VK_LOAD_DEVICE_FUNC(vkCreateFramebuffer);
    VK_LOAD_DEVICE_FUNC(vkDestroyFramebuffer);
    VK_LOAD_DEVICE_FUNC(vkCreateImageView);
    VK_LOAD_DEVICE_FUNC(vkDestroyImageView);
    VK_LOAD_DEVICE_FUNC(vkCmdBeginRenderPass);
    VK_LOAD_DEVICE_FUNC(vkCmdEndRenderPass);
    VK_LOAD_DEVICE_FUNC(vkCmdPipelineBarrier);
    VK_LOAD_DEVICE_FUNC(vkResetCommandPool);

    VK_LOAD_DEVICE_FUNC(vkCreateBuffer);
    VK_LOAD_DEVICE_FUNC(vkDestroyBuffer);
    VK_LOAD_DEVICE_FUNC(vkGetBufferMemoryRequirements);
    VK_LOAD_DEVICE_FUNC(vkBindBufferMemory);
    VK_LOAD_DEVICE_FUNC(vkCmdCopyBuffer);

    VK_LOAD_DEVICE_FUNC(vkCreateImage);
    VK_LOAD_DEVICE_FUNC(vkDestroyImage);
    VK_LOAD_DEVICE_FUNC(vkGetImageMemoryRequirements);

    VK_LOAD_DEVICE_FUNC(vkAllocateMemory);
    VK_LOAD_DEVICE_FUNC(vkFreeMemory);
    VK_LOAD_DEVICE_FUNC(vkBindImageMemory);
    VK_LOAD_DEVICE_FUNC(vkMapMemory);
    VK_LOAD_DEVICE_FUNC(vkUnmapMemory);

    CANVAS_RETURN(CANVAS_OK);
}

static bool vk_check_device_extension_support(VkPhysicalDevice device)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(device);

    uint32_t extension_count = 0;
    vk_info.vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);

    if (extension_count == 0)
    {
        CANVAS_RETURN(false);
    }

    VkExtensionProperties *available_extensions = (VkExtensionProperties *)malloc(extension_count * sizeof(VkExtensionProperties));
    CANVAS_ASSERT_NOT_NULL(available_extensions);
    if (!available_extensions)
    {
        CANVAS_RETURN(false);
    }

    vk_info.vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, available_extensions);

    bool has_swapchain = false;
    for (uint32_t i = 0; i < extension_count; i++)
    {
        if (strcmp(available_extensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            has_swapchain = true;
            break;
        }
    }

    free(available_extensions);
    CANVAS_RETURN(has_swapchain);
}

static bool vk_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface, int *graphics_family, int *present_family)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(device);
    CANVAS_ASSERT_NOT_NULL(surface);
    CANVAS_ASSERT_NOT_NULL(graphics_family);
    CANVAS_ASSERT_NOT_NULL(present_family);

    uint32_t queue_family_count = 0;
    vk_info.vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    if (queue_family_count == 0)
    {
        CANVAS_RETURN(false);
    }

    VkQueueFamilyProperties *queue_families = (VkQueueFamilyProperties *)malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
    CANVAS_ASSERT_NOT_NULL(queue_families);
    if (!queue_families)
    {
        CANVAS_RETURN(false);
    }

    vk_info.vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    *graphics_family = -1;
    *present_family = -1;

    for (uint32_t i = 0; i < queue_family_count; i++)
    {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            *graphics_family = (int)i;

        VkBool32 present_support = VK_FALSE;
        vk_info.vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (present_support)
            *present_family = (int)i;

        if (*graphics_family >= 0 && *present_family >= 0)
            break;
    }

    free(queue_families);
    bool result = (*graphics_family >= 0 && *present_family >= 0);
    CANVAS_RETURN(result);
}

static bool vk_is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR test_surface)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(device);
    CANVAS_ASSERT_NOT_NULL(test_surface);

    int graphics_family, present_family;

    if (!vk_find_queue_families(device, test_surface, &graphics_family, &present_family))
    {
        CANVAS_RETURN(false);
    }

    if (!vk_check_device_extension_support(device))
    {
        CANVAS_RETURN(false);
    }

    uint32_t format_count = 0;
    vk_info.vkGetPhysicalDeviceSurfaceFormatsKHR(device, test_surface, &format_count, NULL);

    uint32_t present_mode_count = 0;
    vk_info.vkGetPhysicalDeviceSurfacePresentModesKHR(device, test_surface, &present_mode_count, NULL);

    bool result = (format_count > 0 && present_mode_count > 0);
    CANVAS_RETURN(result);
}

static VkResult vk_select_physical_device(VkSurfaceKHR test_surface)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(vk_info.instance);
    CANVAS_ASSERT_NOT_NULL(test_surface);

    uint32_t device_count = 0;
    VkResult result = vk_info.vkEnumeratePhysicalDevices(vk_info.instance, &device_count, NULL);
    if (result != VK_SUCCESS || device_count == 0)
    {
        CANVAS_RETURN_ERR(VK_ERROR_INITIALIZATION_FAILED, "No physical devices found or enumeration failed\n");
    }

    VkPhysicalDevice *devices = (VkPhysicalDevice *)malloc(device_count * sizeof(VkPhysicalDevice));
    CANVAS_ASSERT_NOT_NULL(devices);
    if (!devices)
    {
        CANVAS_RETURN(VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    result = vk_info.vkEnumeratePhysicalDevices(vk_info.instance, &device_count, devices);
    if (result != VK_SUCCESS)
    {
        free(devices);
        CANVAS_RETURN(result);
    }

    CANVAS_DBG("Found %u physical devices\n", device_count);

    for (uint32_t i = 0; i < device_count; i++)
    {
        if (vk_is_device_suitable(devices[i], test_surface))
        {
            vk_info.physical_device = devices[i];

            vk_find_queue_families(devices[i], test_surface, &vk_info.graphics_family, &vk_info.present_family);

            VkPhysicalDeviceProperties props;
            vk_info.vkGetPhysicalDeviceProperties(devices[i], &props);
            CANVAS_INFO("selected GPU: %s\n", props.deviceName);

            free(devices);
            CANVAS_RETURN(VK_SUCCESS);
        }
    }

    free(devices);
    CANVAS_RETURN_ERR(VK_ERROR_INITIALIZATION_FAILED, "No suitable physical device found\n");
}

static int vk_create_logical_device()
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(vk_info.physical_device);
    CANVAS_ASSERT(vk_info.graphics_family >= 0);
    CANVAS_ASSERT(vk_info.present_family >= 0);

    uint32_t unique_families[2];
    uint32_t unique_count = 1;
    unique_families[0] = (uint32_t)vk_info.graphics_family;

    if (vk_info.graphics_family != vk_info.present_family)
    {
        unique_families[1] = (uint32_t)vk_info.present_family;
        unique_count = 2;
    }

    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_infos[2];

    for (uint32_t i = 0; i < unique_count; i++)
    {
        queue_create_infos[i] = (VkDeviceQueueCreateInfo){0};
        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = unique_families[i];
        queue_create_infos[i].queueCount = 1;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    VkPhysicalDeviceFeatures device_features = {0};

    const char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = unique_count;
    create_info.pQueueCreateInfos = queue_create_infos;
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = 1;
    create_info.ppEnabledExtensionNames = device_extensions;

    VkResult result = vk_info.vkCreateDevice(vk_info.physical_device, &create_info, NULL, &vk_info.device);
    VK_CHECK(result, "failed to create logical device");

    CANVAS_ASSERT_NOT_NULL(vk_info.device);

    vk_info.vkGetDeviceQueue(vk_info.device, vk_info.graphics_family, 0, &vk_info.graphics_queue);
    vk_info.vkGetDeviceQueue(vk_info.device, vk_info.present_family, 0, &vk_info.present_queue);

    CANVAS_ASSERT_NOT_NULL(vk_info.graphics_queue);
    CANVAS_ASSERT_NOT_NULL(vk_info.present_queue);

    CANVAS_RETURN(CANVAS_OK);
}

static int vk_create_surface(int window_id, VkSurfaceKHR *surface)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);
    CANVAS_ASSERT_NOT_NULL(surface);
    CANVAS_ASSERT_NOT_NULL(canvas_info.canvas[window_id].window);

    VkResult result;

#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = (HWND)canvas_info.canvas[window_id].window;
    create_info.hinstance = GetModuleHandle(NULL);

    result = vk_info.vkCreateWin32SurfaceKHR(vk_info.instance, &create_info, NULL, surface);
    VK_CHECK(result, "failed to create Win32 surface");

#elif defined(__linux__)
    if (_canvas_using_wayland && vk_info.vkCreateWaylandSurfaceKHR)
    {
        VkWaylandSurfaceCreateInfoKHR create_info = {0};
        create_info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;

        result = vk_info.vkCreateWaylandSurfaceKHR(vk_info.instance, &create_info, NULL, surface);
        VK_CHECK(result, "failed to create Wayland surface");
    }
    else if (vk_info.vkCreateXlibSurfaceKHR)
    {
        CANVAS_ASSERT_NOT_NULL(x11.display);

        VkXlibSurfaceCreateInfoKHR create_info = {0};
        create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        create_info.dpy = x11.display;
        create_info.window = (Window)canvas_info.canvas[window_id].window;

        result = vk_info.vkCreateXlibSurfaceKHR(vk_info.instance, &create_info, NULL, surface);
        VK_CHECK(result, "failed to create Xlib surface");
    }
    else
    {
        CANVAS_RETURN_ERR(CANVAS_FAIL, "no surface creation function available for Linux\n");
    }

#elif defined(__APPLE__)
    CANVAS_ASSERT_NOT_NULL(_canvas_data[window_id].layer);
    if (!_canvas_data[window_id].layer)
    {
        CANVAS_RETURN_ERR(CANVAS_FAIL, "no Metal layer available for surface creation\n");
    }

    VkMetalSurfaceCreateInfoEXT create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    create_info.pLayer = _canvas_data[window_id].layer;

    result = vk_info.vkCreateMetalSurfaceEXT(vk_info.instance, &create_info, NULL, surface);
    VK_CHECK(result, "failed to create Metal surface");

#else
    CANVAS_RETURN_ERR(CANVAS_FAIL, "unsupported platform for Vulkan surface creation\n");
#endif

    CANVAS_ASSERT_NOT_NULL(*surface);
    CANVAS_RETURN(CANVAS_OK);
}

int canvas_backend_vulkan_init()
{
    CANVAS_ENTER_FUNC();

    if (vk_info.instance)
    {
        CANVAS_DBG("Vulkan already initialized\n");
        CANVAS_RETURN(CANVAS_OK);
    }

    CANVAS_INFO("initializing Vulkan backend\n");

    vk_info.library = canvas_library_load(vulkan_library_names, canvas_vulkan_names);

    if (!vk_info.library)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_LOAD_LIBRARY, "failed to load Vulkan library\n");
    }

    vk_info.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)canvas_library_symbol(vk_info.library, "vkGetInstanceProcAddr");
    vk_info.vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)canvas_library_symbol(vk_info.library, "vkGetDeviceProcAddr");

    CANVAS_ASSERT_NOT_NULL(vk_info.vkGetInstanceProcAddr);
    if (!vk_info.vkGetInstanceProcAddr)
    {
        CANVAS_ERR("failed to load vkGetInstanceProcAddr\n");
        canvas_library_close(vk_info.library);
        CANVAS_RETURN(CANVAS_ERR_LOAD_SYMBOL);
    }

    vk_info.vkCreateInstance = (PFN_vkCreateInstance)vk_info.vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    vk_info.vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)vk_info.vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceLayerProperties");
    vk_info.vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)vk_info.vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceExtensionProperties");

    CANVAS_ASSERT_NOT_NULL(vk_info.vkCreateInstance);
    CANVAS_ASSERT_NOT_NULL(vk_info.vkEnumerateInstanceLayerProperties);
    CANVAS_ASSERT_NOT_NULL(vk_info.vkEnumerateInstanceExtensionProperties);

    if (!vk_info.vkCreateInstance || !vk_info.vkEnumerateInstanceLayerProperties || !vk_info.vkEnumerateInstanceExtensionProperties)
    {
        CANVAS_ERR("failed to load Vulkan functions\n");
        canvas_library_close(vk_info.library);
        CANVAS_RETURN(CANVAS_ERR_LOAD_SYMBOL);
    }

    CANVAS_INFO("creating vulkan instance...\n");

#ifdef NDEBUG
    vk_info.validation_enabled = false;
#else
    vk_info.validation_enabled = vk_check_validation_layers();
    if (vk_info.validation_enabled)
        CANVAS_INFO("vulkan validation layers enabled\n");
    else
        CANVAS_INFO("vulkan validation layers not available - continuing without validation\n");

#endif

    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Application";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Canvas";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    const char *extensions[16];
    uint32_t extension_count = 0;

    extensions[extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;

#ifdef _WIN32
    extensions[extension_count++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#endif
#ifdef __linux__
    extensions[extension_count++] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
    extensions[extension_count++] = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
#endif
#ifdef __APPLE__
    extensions[extension_count++] = VK_EXT_METAL_SURFACE_EXTENSION_NAME;
    extensions[extension_count++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
#endif

    if (vk_info.validation_enabled)
        extensions[extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    CANVAS_DBG("Enabling %u Vulkan extensions\n", extension_count);

    const char *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {0};

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = extension_count;
    create_info.ppEnabledExtensionNames = extensions;

#ifdef __APPLE__
    create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    if (vk_info.validation_enabled)
    {
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = validation_layers;

        debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_create_info.pfnUserCallback = vk_debug_callback;

        create_info.pNext = &debug_create_info;
    }

    VkResult result = vk_info.vkCreateInstance(&create_info, NULL, &vk_info.instance);

    if (result != VK_SUCCESS)
    {
        CANVAS_RETURN_ERR(result, "vkCreateInstance failed with code: %d\n", result);
    }

    CANVAS_ASSERT_NOT_NULL(vk_info.instance);

    result = vk_load_instance_functions();
    if (result != CANVAS_OK)
    {
        CANVAS_ERR("failed to load vulkan instance functions\n");
        vk_info.vkDestroyInstance(vk_info.instance, NULL);
        canvas_library_close(vk_info.library);
        CANVAS_RETURN(result);
    }

    if (!vk_info.validation_enabled || !vk_info.vkCreateDebugUtilsMessengerEXT)
    {
        CANVAS_RETURN(CANVAS_OK);
    }

    VkDebugUtilsMessengerCreateInfoEXT create_info_dbg = {0};
    create_info_dbg.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info_dbg.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info_dbg.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info_dbg.pfnUserCallback = vk_debug_callback;

    vk_info.vkCreateDebugUtilsMessengerEXT(vk_info.instance, &create_info_dbg, NULL, &vk_info.debug_messenger);

    CANVAS_RETURN(CANVAS_OK);
}

static void vk_cleanup_swapchain_support_details(SwapchainSupportDetails *details)
{
    CANVAS_ASSERT_NOT_NULL(details);
    if (details->formats)
        free(details->formats);
    if (details->present_modes)
        free(details->present_modes);
}

static VkExtent2D vk_choose_swap_extent(const VkSurfaceCapabilitiesKHR *capabilities, int window_id)
{
    CANVAS_ASSERT_NOT_NULL(capabilities);
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);

    if (capabilities->currentExtent.width != UINT32_MAX)
        return capabilities->currentExtent;

    VkExtent2D actual_extent = {
        .width = (uint32_t)canvas_info.canvas[window_id].width,
        .height = (uint32_t)canvas_info.canvas[window_id].height};

    actual_extent.width = actual_extent.width < capabilities->minImageExtent.width
                              ? capabilities->minImageExtent.width
                              : (actual_extent.width > capabilities->maxImageExtent.width
                                     ? capabilities->maxImageExtent.width
                                     : actual_extent.width);

    actual_extent.height = actual_extent.height < capabilities->minImageExtent.height
                               ? capabilities->minImageExtent.height
                               : (actual_extent.height > capabilities->maxImageExtent.height
                                      ? capabilities->maxImageExtent.height
                                      : actual_extent.height);

    return actual_extent;
}

static int vk_create_swapchain(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];
    CANVAS_ASSERT_NOT_NULL(vk_win->surface);

    SwapchainSupportDetails support = {0};

    vk_info.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_info.physical_device, vk_win->surface, &support.capabilities);

    vk_info.vkGetPhysicalDeviceSurfaceFormatsKHR(vk_info.physical_device, vk_win->surface, &support.format_count, NULL);
    if (support.format_count > 0)
    {
        support.formats = (VkSurfaceFormatKHR *)malloc(support.format_count * sizeof(VkSurfaceFormatKHR));
        CANVAS_ASSERT_NOT_NULL(support.formats);
        vk_info.vkGetPhysicalDeviceSurfaceFormatsKHR(vk_info.physical_device, vk_win->surface, &support.format_count, support.formats);
    }

    vk_info.vkGetPhysicalDeviceSurfacePresentModesKHR(vk_info.physical_device, vk_win->surface, &support.present_mode_count, NULL);
    if (support.present_mode_count > 0)
    {
        support.present_modes = (VkPresentModeKHR *)malloc(support.present_mode_count * sizeof(VkPresentModeKHR));
        CANVAS_ASSERT_NOT_NULL(support.present_modes);
        vk_info.vkGetPhysicalDeviceSurfacePresentModesKHR(vk_info.physical_device, vk_win->surface, &support.present_mode_count, support.present_modes);
    }

    if (support.format_count == 0 || support.present_mode_count == 0)
    {
        vk_cleanup_swapchain_support_details(&support);
        CANVAS_RETURN_ERR(CANVAS_FAIL, "inadequate swapchain support\n");
    }

    VkSurfaceFormatKHR surface_format = support.formats[0];
    for (uint32_t i = 0; i < support.format_count; i++)
        if (support.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && support.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            surface_format = support.formats[i];

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < support.present_mode_count; i++)
        if (support.present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            present_mode = VK_PRESENT_MODE_MAILBOX_KHR;

    for (uint32_t i = 0; i < support.present_mode_count; i++)
        if (support.present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
            present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;

    VkExtent2D extent = vk_choose_swap_extent(&support.capabilities, window_id);

    CANVAS_ASSERT(extent.width > 0);
    CANVAS_ASSERT(extent.height > 0);

    canvas_info.canvas[window_id].width = (int)extent.width;
    canvas_info.canvas[window_id].height = (int)extent.height;

    uint32_t image_count = support.capabilities.minImageCount + 1;

    if (support.capabilities.maxImageCount > 0 && image_count > support.capabilities.maxImageCount)
        image_count = support.capabilities.maxImageCount;

    if (image_count > MAX_SWAPCHAIN_IMAGES)
        image_count = MAX_SWAPCHAIN_IMAGES;

    CANVAS_ASSERT_RANGE(image_count, 1, MAX_SWAPCHAIN_IMAGES);

    VkSwapchainCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = vk_win->surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queue_families[] = {(uint32_t)vk_info.graphics_family, (uint32_t)vk_info.present_family};

    if (vk_info.graphics_family != vk_info.present_family)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_families;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vk_info.vkCreateSwapchainKHR(vk_info.device, &create_info, NULL, &vk_win->swapchain);

    vk_cleanup_swapchain_support_details(&support);

    VK_CHECK(result, "failed to create swapchain");
    CANVAS_ASSERT_NOT_NULL(vk_win->swapchain);

    vk_info.vkGetSwapchainImagesKHR(vk_info.device, vk_win->swapchain, &vk_win->swapchain_image_count, NULL);

    if (vk_win->swapchain_image_count > MAX_SWAPCHAIN_IMAGES)
        vk_win->swapchain_image_count = MAX_SWAPCHAIN_IMAGES;

    CANVAS_ASSERT_RANGE(vk_win->swapchain_image_count, 1, MAX_SWAPCHAIN_IMAGES);

    vk_info.vkGetSwapchainImagesKHR(vk_info.device, vk_win->swapchain, &vk_win->swapchain_image_count, vk_win->swapchain_images);

    vk_win->swapchain_format = surface_format.format;
    vk_win->swapchain_extent = extent;

    for (uint32_t i = 0; i < vk_win->swapchain_image_count; i++)
    {
        CANVAS_ASSERT_NOT_NULL(vk_win->swapchain_images[i]);

        VkImageViewCreateInfo create_info = {0};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = vk_win->swapchain_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = vk_win->swapchain_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        VkResult result = vk_info.vkCreateImageView(vk_info.device, &create_info, NULL, &vk_win->swapchain_image_views[i]);
        if (result != VK_SUCCESS)
        {
            CANVAS_ERR("failed to create image view %u\n", i);

            for (uint32_t j = 0; j < i; j++)
                vk_info.vkDestroyImageView(vk_info.device, vk_win->swapchain_image_views[j], NULL);

            CANVAS_RETURN(CANVAS_FAIL);
        }
        CANVAS_ASSERT_NOT_NULL(vk_win->swapchain_image_views[i]);
    }

    CANVAS_VERBOSE("swapchain created: %ux%u, %u images\n", extent.width, extent.height, vk_win->swapchain_image_count);

    CANVAS_RETURN(CANVAS_OK);
}

static int vk_create_render_pass(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    VkAttachmentDescription color_attachment = {0};
    color_attachment.format = vk_win->swapchain_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {0};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    VkResult result = vk_info.vkCreateRenderPass(vk_info.device, &render_pass_info, NULL, &vk_win->render_pass);
    VK_CHECK(result, "failed to create render pass");

    CANVAS_ASSERT_NOT_NULL(vk_win->render_pass);

    CANVAS_RETURN(CANVAS_OK);
}

static int vk_create_framebuffers(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];
    CANVAS_ASSERT_NOT_NULL(vk_win->render_pass);
    CANVAS_ASSERT(vk_win->swapchain_image_count > 0);

    for (uint32_t i = 0; i < vk_win->swapchain_image_count; i++)
    {
        CANVAS_ASSERT_NOT_NULL(vk_win->swapchain_image_views[i]);

        VkImageView attachments[] = {vk_win->swapchain_image_views[i]};

        VkFramebufferCreateInfo framebuffer_info = {0};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = vk_win->render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = vk_win->swapchain_extent.width;
        framebuffer_info.height = vk_win->swapchain_extent.height;
        framebuffer_info.layers = 1;

        VkResult result = vk_info.vkCreateFramebuffer(vk_info.device, &framebuffer_info, NULL, &vk_win->framebuffers[i]);
        if (result != VK_SUCCESS)
        {
            CANVAS_ERR("failed to create framebuffer %u\n", i);

            for (uint32_t j = 0; j < i; j++)
                vk_info.vkDestroyFramebuffer(vk_info.device, vk_win->framebuffers[j], NULL);

            CANVAS_RETURN(CANVAS_FAIL);
        }
        CANVAS_ASSERT_NOT_NULL(vk_win->framebuffers[i]);
    }

    CANVAS_RETURN(CANVAS_OK);
}

static int vk_create_command_pool(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);
    CANVAS_ASSERT(vk_info.graphics_family >= 0);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    VkCommandPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = (uint32_t)vk_info.graphics_family;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult result = vk_info.vkCreateCommandPool(vk_info.device, &pool_info, NULL, &vk_win->command_pool);
    VK_CHECK(result, "failed to create command pool");

    CANVAS_ASSERT_NOT_NULL(vk_win->command_pool);

    CANVAS_RETURN(CANVAS_OK);
}

static int vk_create_command_buffers(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];
    CANVAS_ASSERT_NOT_NULL(vk_win->command_pool);
    CANVAS_ASSERT(vk_win->swapchain_image_count > 0);

    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = vk_win->command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = vk_win->swapchain_image_count;

    VkResult result = vk_info.vkAllocateCommandBuffers(vk_info.device, &alloc_info, vk_win->command_buffers);
    VK_CHECK(result, "failed to allocate command buffers");

    for (uint32_t i = 0; i < vk_win->swapchain_image_count; i++)
    {
        CANVAS_ASSERT_NOT_NULL(vk_win->command_buffers[i]);
    }

    CANVAS_RETURN(CANVAS_OK);
}

static int vk_create_sync_objects(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);
    CANVAS_ASSERT_NOT_NULL(vk_info.device);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    VkSemaphoreCreateInfo semaphore_info = {0};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {0};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
    {
        VkResult result;

        result = vk_info.vkCreateSemaphore(vk_info.device, &semaphore_info, NULL,
                                           &vk_win->image_available_semaphores[i]);
        if (result != VK_SUCCESS)
        {
            CANVAS_ERR("failed to create image available semaphore %u (result=%d)\n", i, result);
            goto cleanup_semaphores;
        }

        result = vk_info.vkCreateSemaphore(vk_info.device, &semaphore_info, NULL,
                                           &vk_win->render_finished_semaphores[i]);
        if (result != VK_SUCCESS)
        {
            CANVAS_ERR("failed to create render finished semaphore %u (result=%d)\n", i, result);
            vk_info.vkDestroySemaphore(vk_info.device, vk_win->image_available_semaphores[i], NULL);
            goto cleanup_semaphores;
        }
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkResult result = vk_info.vkCreateFence(vk_info.device, &fence_info, NULL,
                                                &vk_win->in_flight_fences[i]);
        if (result != VK_SUCCESS)
        {
            CANVAS_ERR("failed to create fence %d (result=%d)\n", i, result);
            goto cleanup_fences;
        }
    }

    for (uint32_t i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
        vk_win->images_in_flight[i] = VK_NULL_HANDLE;

    CANVAS_RETURN(CANVAS_OK);

cleanup_fences:
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vk_win->in_flight_fences[i])
            vk_info.vkDestroyFence(vk_info.device, vk_win->in_flight_fences[i], NULL);
    }

cleanup_semaphores:
    for (uint32_t i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
    {
        if (vk_win->image_available_semaphores[i])
            vk_info.vkDestroySemaphore(vk_info.device, vk_win->image_available_semaphores[i], NULL);
        if (vk_win->render_finished_semaphores[i])
            vk_info.vkDestroySemaphore(vk_info.device, vk_win->render_finished_semaphores[i], NULL);
    }
    CANVAS_RETURN(CANVAS_FAIL);
}

static void vk_cleanup_swapchain(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    if (!vk_win->initialized)
    {
        CANVAS_RETURN_VOID();
    }

    if (vk_win->command_pool && vk_win->swapchain_image_count > 0)
    {
        vk_info.vkFreeCommandBuffers(vk_info.device, vk_win->command_pool,
                                     vk_win->swapchain_image_count, vk_win->command_buffers);

        for (uint32_t i = 0; i < vk_win->swapchain_image_count; i++)
            vk_win->command_buffers[i] = VK_NULL_HANDLE;
    }

    for (uint32_t i = 0; i < vk_win->swapchain_image_count; i++)
    {
        if (vk_win->framebuffers[i])
        {
            vk_info.vkDestroyFramebuffer(vk_info.device, vk_win->framebuffers[i], NULL);
            vk_win->framebuffers[i] = VK_NULL_HANDLE;
        }
        if (vk_win->swapchain_image_views[i])
        {
            vk_info.vkDestroyImageView(vk_info.device, vk_win->swapchain_image_views[i], NULL);
            vk_win->swapchain_image_views[i] = VK_NULL_HANDLE;
        }
    }

    for (uint32_t i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
        vk_win->images_in_flight[i] = VK_NULL_HANDLE;

    if (vk_win->swapchain)
    {
        vk_info.vkDestroySwapchainKHR(vk_info.device, vk_win->swapchain, NULL);
        vk_win->swapchain = VK_NULL_HANDLE;
    }

    CANVAS_RETURN_VOID();
}

static int vk_recreate_swapchain(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);
    CANVAS_ASSERT_NOT_NULL(vk_info.device);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vk_win->in_flight_fences[i] != VK_NULL_HANDLE)
        {
            vk_info.vkWaitForFences(vk_info.device, 1, &vk_win->in_flight_fences[i], VK_TRUE, UINT64_MAX);
        }
    }

    vk_info.vkQueueWaitIdle(vk_info.graphics_queue);
    vk_info.vkQueueWaitIdle(vk_info.present_queue);

    vk_cleanup_swapchain(window_id);

    int result;
    result = vk_create_swapchain(window_id);
    if (result != CANVAS_OK)
    {
        CANVAS_RETURN_ERR(result, "vk_recreate_swapchain: Failed to create swapchain\n");
    }

    result = vk_create_framebuffers(window_id);
    if (result != CANVAS_OK)
    {
        CANVAS_RETURN_ERR(result, "vk_recreate_swapchain: Failed to create framebuffers\n");
    }

    result = vk_create_command_buffers(window_id);
    if (result != CANVAS_OK)
    {
        CANVAS_RETURN_ERR(result, "vk_recreate_swapchain: Failed to create command buffers\n");
    }

    vk_win->needs_resize = false;
    vk_win->current_frame = 0;

    CANVAS_RETURN(CANVAS_OK);
}

static int vk_record_command_buffer(int window_id, uint32_t image_index)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];
    CANVAS_ASSERT(image_index < vk_win->swapchain_image_count);
    CANVAS_ASSERT_NOT_NULL(vk_win->command_buffers[image_index]);
    CANVAS_ASSERT_NOT_NULL(vk_win->render_pass);
    CANVAS_ASSERT_NOT_NULL(vk_win->framebuffers[image_index]);

    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkResult result = vk_info.vkBeginCommandBuffer(vk_win->command_buffers[image_index], &begin_info);
    if (result != VK_SUCCESS)
    {
        CANVAS_RETURN_ERR(CANVAS_FAIL, "failed to begin command buffer (result=%d)\n", result);
    }

    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = vk_win->render_pass;
    render_pass_info.framebuffer = vk_win->framebuffers[image_index];
    render_pass_info.renderArea.offset = (VkOffset2D){0, 0};
    render_pass_info.renderArea.extent = vk_win->swapchain_extent;

    VkClearValue clear_color = {0};
    clear_color.color.float32[0] = canvas_info.canvas[window_id].clear[0];
    clear_color.color.float32[1] = canvas_info.canvas[window_id].clear[1];
    clear_color.color.float32[2] = canvas_info.canvas[window_id].clear[2];
    clear_color.color.float32[3] = canvas_info.canvas[window_id].clear[3];

    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vk_info.vkCmdBeginRenderPass(vk_win->command_buffers[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vk_info.vkCmdEndRenderPass(vk_win->command_buffers[image_index]);

    result = vk_info.vkEndCommandBuffer(vk_win->command_buffers[image_index]);
    if (result != VK_SUCCESS)
    {
        CANVAS_RETURN_ERR(CANVAS_FAIL, "failed to end command buffer (result=%d)\n", result);
    }

    CANVAS_RETURN(CANVAS_OK);
}

static int vk_draw_frame(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    if (!vk_win->initialized)
    {
        CANVAS_RETURN(CANVAS_OK);
    }

    CANVAS_ASSERT_NOT_NULL(vk_info.device);
    CANVAS_ASSERT_NOT_NULL(vk_win->swapchain);

    uint32_t frame_index = vk_win->current_frame % MAX_FRAMES_IN_FLIGHT;
    VkFence current_fence = vk_win->in_flight_fences[frame_index];

    if (current_fence == VK_NULL_HANDLE)
    {
        CANVAS_RETURN(CANVAS_FAIL);
    }

    vk_info.vkWaitForFences(vk_info.device, 1, &current_fence, VK_TRUE, UINT64_MAX);

    uint32_t image_index;

    VkResult result = vk_info.vkAcquireNextImageKHR(
        vk_info.device, vk_win->swapchain, UINT64_MAX,
        vk_win->image_available_semaphores[frame_index],
        VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        int recreate_result = vk_recreate_swapchain(window_id);
        CANVAS_RETURN(recreate_result);
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        CANVAS_RETURN_ERR(CANVAS_FAIL, "failed to acquire swapchain image\n");
    }

    if (image_index >= vk_win->swapchain_image_count)
    {
        CANVAS_RETURN(CANVAS_FAIL);
    }

    if (vk_win->images_in_flight[image_index] != VK_NULL_HANDLE)
    {
        vk_info.vkWaitForFences(vk_info.device, 1, &vk_win->images_in_flight[image_index], VK_TRUE, UINT64_MAX);
    }

    vk_info.vkResetFences(vk_info.device, 1, &current_fence);
    vk_win->images_in_flight[image_index] = current_fence;

    int record_result = vk_record_command_buffer(window_id, image_index);

    if (record_result != CANVAS_OK)
    {
        vk_win->images_in_flight[image_index] = VK_NULL_HANDLE;
        CANVAS_RETURN(record_result);
    }

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {vk_win->image_available_semaphores[frame_index]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &vk_win->command_buffers[image_index];

    VkSemaphore signal_semaphores[] = {vk_win->render_finished_semaphores[image_index]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    result = vk_info.vkQueueSubmit(vk_info.graphics_queue, 1, &submit_info, current_fence);
    if (result != VK_SUCCESS)
    {
        CANVAS_ERR("failed to submit draw command buffer\n");
        vk_win->images_in_flight[image_index] = VK_NULL_HANDLE;
        CANVAS_RETURN(CANVAS_FAIL);
    }

    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapchains[] = {vk_win->swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;

    result = vk_info.vkQueuePresentKHR(vk_info.present_queue, &present_info);

    vk_win->current_frame = (vk_win->current_frame + 1);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vk_win->needs_resize)
    {
        vk_win->needs_resize = false;
        int recreate_result = vk_recreate_swapchain(window_id);
        CANVAS_RETURN(recreate_result);
    }
    else if (result != VK_SUCCESS)
    {
        CANVAS_RETURN_ERR(CANVAS_FAIL, "failed to present swapchain image\n");
    }

    CANVAS_RETURN(CANVAS_OK);
}

static uint32_t vk_find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(vk_info.physical_device);

    VkPhysicalDeviceMemoryProperties mem_props;
    vk_info.vkGetPhysicalDeviceMemoryProperties(vk_info.physical_device, &mem_props);

    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++)
    {
        if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties)
        {
            CANVAS_RETURN(i);
        }
    }

    CANVAS_RETURN_ERR(UINT32_MAX, "Failed to find suitable memory type\n");
}

static int _vulkan_upload_buffer_data(canvas_buffer *buf, void *data, size_t size)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(buf);
    CANVAS_ASSERT_NOT_NULL(data);
    CANVAS_ASSERT(size > 0);
    CANVAS_ASSERT_NOT_NULL(vk_info.device);

    VkBufferCreateInfo staging_info = {0};
    staging_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    staging_info.size = size;
    staging_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    staging_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer staging_buffer;
    VkResult result = vk_info.vkCreateBuffer(vk_info.device, &staging_info, NULL, &staging_buffer);
    if (result != VK_SUCCESS)
    {
        CANVAS_RETURN_ERR(CANVAS_FAIL, "Failed to create staging buffer\n");
    }

    VkMemoryRequirements mem_reqs;
    vk_info.vkGetBufferMemoryRequirements(vk_info.device, staging_buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = vk_find_memory_type(
        mem_reqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (alloc_info.memoryTypeIndex == UINT32_MAX)
    {
        vk_info.vkDestroyBuffer(vk_info.device, staging_buffer, NULL);
        CANVAS_RETURN_ERR(CANVAS_FAIL, "Failed to find suitable memory type for staging buffer\n");
    }

    VkDeviceMemory staging_memory;
    result = vk_info.vkAllocateMemory(vk_info.device, &alloc_info,
                                      NULL, &staging_memory);
    if (result != VK_SUCCESS)
    {
        vk_info.vkDestroyBuffer(vk_info.device, staging_buffer, NULL);
        CANVAS_RETURN_ERR(CANVAS_FAIL, "Failed to allocate staging memory\n");
    }

    vk_info.vkBindBufferMemory(vk_info.device, staging_buffer, staging_memory, 0);

    // Copy data to staging
    void *mapped;
    result = vk_info.vkMapMemory(vk_info.device, staging_memory, 0, size, 0, &mapped);
    if (result != VK_SUCCESS)
    {
        vk_info.vkFreeMemory(vk_info.device, staging_memory, NULL);
        vk_info.vkDestroyBuffer(vk_info.device, staging_buffer, NULL);
        CANVAS_RETURN_ERR(CANVAS_FAIL, "Failed to map staging memory\n");
    }

    memcpy(mapped, data, size);
    vk_info.vkUnmapMemory(vk_info.device, staging_memory);

    // Create one-time command buffer
    canvas_vulkan_window *vk_win = &vk_windows[buf->window_id];

    VkCommandBufferAllocateInfo cmd_alloc_info = {0};
    cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_alloc_info.commandPool = vk_win->command_pool;
    cmd_alloc_info.commandBufferCount = 1;

    VkCommandBuffer cmd_buffer;
    vk_info.vkAllocateCommandBuffers(vk_info.device, &cmd_alloc_info, &cmd_buffer);

    // Begin recording
    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vk_info.vkBeginCommandBuffer(cmd_buffer, &begin_info);

    // Copy command
    VkBufferCopy copy_region = {0};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;

    vk_info.vkCmdCopyBuffer(cmd_buffer, staging_buffer,
                            (VkBuffer)buf->platform_handle, 1, &copy_region);

    vk_info.vkEndCommandBuffer(cmd_buffer);

    // Submit and wait
    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buffer;

    vk_info.vkQueueSubmit(vk_info.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk_info.vkQueueWaitIdle(vk_info.graphics_queue);

    // Cleanup
    vk_info.vkFreeCommandBuffers(vk_info.device, vk_win->command_pool, 1, &cmd_buffer);
    vk_info.vkFreeMemory(vk_info.device, staging_memory, NULL);
    vk_info.vkDestroyBuffer(vk_info.device, staging_buffer, NULL);

    CANVAS_RETURN(CANVAS_OK);
}

canvas_buffer *canvas_buffer_create(int window_id, canvas_buffer_type type, canvas_buffer_usage usage, size_t size, void *initial_data)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);
    CANVAS_ASSERT(size > 0);
    CANVAS_ASSERT_RANGE(type, CANVAS_BUFFER_VERTEX, CANVAS_BUFFER_STORAGE);
    CANVAS_ASSERT_RANGE(usage, CANVAS_BUFFER_STATIC, CANVAS_BUFFER_STAGING);

    CANVAS_VALID_PTR(window_id);

    CANVAS_ASSERT_NOT_NULL(vk_info.device);
    CANVAS_ASSERT_NOT_NULL(vk_info.vkCreateBuffer);
    CANVAS_ASSERT_NOT_NULL(vk_info.vkAllocateMemory);
    CANVAS_ASSERT_NOT_NULL(vk_info.vkBindBufferMemory);

    if (!vk_info.device)
    {
        CANVAS_RETURN_ERR(NULL, "GPU not initialized\\n");
    }

    canvas_buffer *buf = (canvas_buffer *)calloc(1, sizeof(canvas_buffer));
    if (!buf)
    {
        CANVAS_RETURN(NULL);
    }
    CANVAS_ASSERT_NOT_NULL(buf);

    buf->size = size;
    buf->type = type;
    buf->usage = usage;
    buf->window_id = window_id;

    // Determine usage flags
    VkBufferUsageFlags usage_flags;
    switch (type)
    {
    case CANVAS_BUFFER_VERTEX:
        usage_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case CANVAS_BUFFER_INDEX:
        usage_flags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case CANVAS_BUFFER_UNIFORM:
        usage_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        break;
    case CANVAS_BUFFER_STORAGE:
        usage_flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        break;
    default:
        usage_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    if (initial_data)
        usage_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    // Create buffer
    VkBufferCreateInfo buffer_info = {0};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage_flags;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer vk_buffer;
    VkResult result = vk_info.vkCreateBuffer(vk_info.device, &buffer_info,
                                             NULL, &vk_buffer);
    if (result != VK_SUCCESS)
    {
        CANVAS_ERR("Failed to create Vulkan buffer\n");
        free(buf);
        CANVAS_RETURN(NULL);
    }

    // Get memory requirements
    VkMemoryRequirements mem_reqs;
    vk_info.vkGetBufferMemoryRequirements(vk_info.device, vk_buffer, &mem_reqs);

    // Determine memory properties
    VkMemoryPropertyFlags mem_props;
    if (usage == CANVAS_BUFFER_DYNAMIC)
    {
        mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    else
    {
        mem_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }

    // Allocate memory
    VkMemoryAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = vk_find_memory_type(mem_reqs.memoryTypeBits, mem_props);

    if (alloc_info.memoryTypeIndex == UINT32_MAX)
    {
        CANVAS_ERR("Failed to find suitable memory type for buffer\n");
        vk_info.vkDestroyBuffer(vk_info.device, vk_buffer, NULL);
        free(buf);
        CANVAS_RETURN(NULL);
    }

    VkDeviceMemory memory;
    result = vk_info.vkAllocateMemory(vk_info.device, &alloc_info, NULL, &memory);
    if (result != VK_SUCCESS)
    {
        CANVAS_ERR("Failed to allocate Vulkan buffer memory\n");
        vk_info.vkDestroyBuffer(vk_info.device, vk_buffer, NULL);
        free(buf);
        CANVAS_RETURN(NULL);
    }

    // Bind memory
    vk_info.vkBindBufferMemory(vk_info.device, vk_buffer, memory, 0);

    buf->platform_handle = vk_buffer;
    buf->memory = (uint64_t)memory;

    if (usage == CANVAS_BUFFER_DYNAMIC)
    {
        result = vk_info.vkMapMemory(vk_info.device, memory, 0, size,
                                     0, &buf->mapped);
        if (result != VK_SUCCESS)
        {
            CANVAS_ERR("Failed to map Vulkan buffer\n");
            vk_info.vkFreeMemory(vk_info.device, memory, NULL);
            vk_info.vkDestroyBuffer(vk_info.device, vk_buffer, NULL);
            free(buf);
            CANVAS_RETURN(NULL);
        }

        if (initial_data)
        {
            memcpy(buf->mapped, initial_data, size);
        }
    }
    else if (initial_data)
    {
        if (_vulkan_upload_buffer_data(buf, initial_data, size) != CANVAS_OK)
        {
            CANVAS_ERR("Failed to upload device-local buffer data\n");
            vk_info.vkFreeMemory(vk_info.device, memory, NULL);
            vk_info.vkDestroyBuffer(vk_info.device, vk_buffer, NULL);
            free(buf);
            CANVAS_RETURN(NULL);
        }
    }

    CANVAS_RETURN(buf);
}

void canvas_buffer_update(canvas_buffer *buf, void *data, size_t size, size_t offset)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(buf);
    CANVAS_ASSERT_NOT_NULL(data);
    CANVAS_ASSERT(size > 0);

    if (!buf || !data)
    {
        CANVAS_RETURN_VOID();
    }

    CANVAS_ASSERT_RANGE(buf->type, CANVAS_BUFFER_VERTEX, CANVAS_BUFFER_STORAGE);
    CANVAS_ASSERT_RANGE(buf->usage, CANVAS_BUFFER_STATIC, CANVAS_BUFFER_STAGING);
    CANVAS_ASSERT(buf->size > 0);
    CANVAS_ASSERT(offset + size <= buf->size);

    if (buf->usage != CANVAS_BUFFER_DYNAMIC)
    {
        CANVAS_WARN("Can only update dynamic buffers\\n");
        CANVAS_RETURN_VOID();
    }

    CANVAS_ASSERT_NOT_NULL(buf->mapped);

    if (offset > buf->size || size > buf->size - offset)
    {
        CANVAS_ERR("Buffer update out of bounds\\n");
        CANVAS_RETURN_VOID();
    }

    memcpy((uint8_t *)buf->mapped + offset, data, size);
    CANVAS_RETURN_VOID();
}

void *canvas_buffer_map(canvas_buffer *buf)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(buf);

    if (!buf)
    {
        CANVAS_RETURN(NULL);
    }

    CANVAS_ASSERT(buf->size > 0);
    CANVAS_ASSERT_NOT_NULL(vk_info.device);

    if (buf->usage == CANVAS_BUFFER_DYNAMIC)
    {
        CANVAS_ASSERT_NOT_NULL(buf->mapped);
        CANVAS_RETURN(buf->mapped); // Already mapped
    }

    // Map device-local buffers on demand (not recommended)
    void *mapped = NULL;
    CANVAS_ASSERT_NOT_NULL(vk_info.vkMapMemory);
    VkResult result = vk_info.vkMapMemory(vk_info.device, (VkDeviceMemory)buf->memory, 0, buf->size, 0, &mapped);
    if (result != VK_SUCCESS)
    {
        CANVAS_RETURN_ERR(NULL, "Failed to map Vulkan buffer\\n");
    }

    CANVAS_RETURN(mapped);
}

void canvas_buffer_unmap(canvas_buffer *buf)
{
    CANVAS_ENTER_FUNC();

    if (!buf)
    {
        CANVAS_RETURN_VOID();
    }

    if (buf->usage == CANVAS_BUFFER_DYNAMIC)
    {
        CANVAS_RETURN_VOID(); // Persistently mapped
    }

    vk_info.vkUnmapMemory(vk_info.device, (VkDeviceMemory)buf->memory);
    CANVAS_RETURN_VOID();
}

void canvas_buffer_destroy(canvas_buffer *buf)
{
    CANVAS_ENTER_FUNC();

    if (!buf)
    {
        CANVAS_RETURN_VOID();
    }

    CANVAS_ASSERT_NOT_NULL(vk_info.device);
    CANVAS_ASSERT_NOT_NULL(vk_info.vkDestroyBuffer);
    CANVAS_ASSERT_NOT_NULL(vk_info.vkFreeMemory);
    CANVAS_ASSERT(buf->size > 0);
    CANVAS_ASSERT_RANGE(buf->type, CANVAS_BUFFER_VERTEX, CANVAS_BUFFER_STORAGE);

    VkBuffer vk_buffer = (VkBuffer)buf->platform_handle;
    CANVAS_ASSERT_NOT_NULL(vk_buffer);

    if (buf->mapped && buf->usage == CANVAS_BUFFER_DYNAMIC)
    {
        CANVAS_ASSERT_NOT_NULL(vk_info.vkUnmapMemory);
        vk_info.vkUnmapMemory(vk_info.device, (VkDeviceMemory)buf->memory);
    }

    vk_info.vkDestroyBuffer(vk_info.device, vk_buffer, NULL);
    vk_info.vkFreeMemory(vk_info.device, (VkDeviceMemory)buf->memory, NULL);

    free(buf);
    CANVAS_RETURN_VOID();
}

static void vk_cleanup_window(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    if (!vk_win->initialized)
    {
        CANVAS_RETURN_VOID();
    }

    CANVAS_ASSERT_NOT_NULL(vk_info.device);

    vk_info.vkDeviceWaitIdle(vk_info.device);

    if (vk_win->depth_view)
    {
        vk_info.vkDestroyImageView(vk_info.device, vk_win->depth_view, NULL);
        vk_win->depth_view = VK_NULL_HANDLE;
    }
    if (vk_win->depth_image)
    {
        vk_info.vkDestroyImage(vk_info.device, vk_win->depth_image, NULL);
        vk_win->depth_image = VK_NULL_HANDLE;
    }
    if (vk_win->depth_memory)
    {
        vk_info.vkFreeMemory(vk_info.device, vk_win->depth_memory, NULL);
        vk_win->depth_memory = VK_NULL_HANDLE;
    }

    for (uint32_t i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
    {
        if (vk_win->image_available_semaphores[i])
            vk_info.vkDestroySemaphore(vk_info.device, vk_win->image_available_semaphores[i], NULL);

        if (vk_win->render_finished_semaphores[i])
            vk_info.vkDestroySemaphore(vk_info.device, vk_win->render_finished_semaphores[i], NULL);
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vk_win->in_flight_fences[i])
            vk_info.vkDestroyFence(vk_info.device, vk_win->in_flight_fences[i], NULL);
    }

    if (vk_win->command_pool)
        vk_info.vkDestroyCommandPool(vk_info.device, vk_win->command_pool, NULL);

    if (vk_win->render_pass)
        vk_info.vkDestroyRenderPass(vk_info.device, vk_win->render_pass, NULL);

    vk_cleanup_swapchain(window_id);

    if (vk_win->surface)
        vk_info.vkDestroySurfaceKHR(vk_info.instance, vk_win->surface, NULL);

    memset(vk_win, 0, sizeof(canvas_vulkan_window));
    CANVAS_RETURN_VOID();
}

static void vk_cleanup(void)
{
    CANVAS_ENTER_FUNC();

    if (!vk_info.instance)
    {
        CANVAS_RETURN_VOID();
    }

    if (vk_info.device)
        vk_info.vkDeviceWaitIdle(vk_info.device);

    for (int i = 0; i < MAX_CANVAS; i++)
        vk_cleanup_window(i);

    if (vk_info.device)
    {
        vk_info.vkDestroyDevice(vk_info.device, NULL);
        vk_info.device = VK_NULL_HANDLE;
    }

    if (vk_info.validation_enabled && vk_info.debug_messenger && vk_info.vkDestroyDebugUtilsMessengerEXT)
    {
        vk_info.vkDestroyDebugUtilsMessengerEXT(vk_info.instance, vk_info.debug_messenger, NULL);
        vk_info.debug_messenger = VK_NULL_HANDLE;
    }

    if (vk_info.instance)
    {
        vk_info.vkDestroyInstance(vk_info.instance, NULL);
        vk_info.instance = VK_NULL_HANDLE;
    }

    if (vk_info.library)
    {
        canvas_library_close(vk_info.library);
        vk_info.library = NULL;
    }

    CANVAS_INFO("vulkan cleaned up\n");
    CANVAS_RETURN_VOID();
}

#endif // CANVAS_VULKAN

//
//
//
#if defined(__APPLE__)

int canvas_minimize(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    objc_id window = canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to minimize: %d\n", window_id);
    }

    msg_void_id(window, sel_c("miniaturize:"), NULL);

    canvas_info.canvas[window_id].minimized = true;
    canvas_info.canvas[window_id].maximized = false;

    CANVAS_RETURN(CANVAS_OK);
}

int canvas_maximize(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    objc_id window = canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to maximize: %d\n", window_id);
    }

    bool is_zoomed = msg_bool(window, "isZoomed");

    if (!is_zoomed)
    {
        msg_void_id(window, sel_c("zoom:"), NULL);
        canvas_info.canvas[window_id].maximized = true;
    }

    canvas_info.canvas[window_id].minimized = false;

    CANVAS_RETURN(CANVAS_OK);
}

int canvas_restore(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    objc_id window = canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to restore: %d\n", window_id);
    }

    if (canvas_info.canvas[window_id].minimized)
    {
        msg_void_id(window, sel_c("deminiaturize:"), NULL);
    }
    else if (canvas_info.canvas[window_id].maximized)
    {
        msg_void_id(window, sel_c("zoom:"), NULL);
    }
    else if (canvas_info.canvas[window_id].fullscreen)
    {
        msg_void_id(window, sel_c("toggleFullScreen:"), NULL);
    }

    canvas_info.canvas[window_id].minimized = false;
    canvas_info.canvas[window_id].maximized = false;
    canvas_info.canvas[window_id].fullscreen = false;

    CANVAS_RETURN(CANVAS_OK);
}

int canvas_fullscreen(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    objc_id window = canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window for fullscreen: %d\n", window_id);
    }

    unsigned long style_mask = msg_ulong(window, "styleMask");
    bool is_fullscreen = (style_mask & (1 << 14)) != 0; // NSWindowStyleMaskFullScreen

    if (!is_fullscreen)
    {
        _CGRect frame = msg_rect(window, "frame");
        _canvas_data[window_id].saved_x = (int)frame.x;
        _canvas_data[window_id].saved_y = (int)frame.y;
        _canvas_data[window_id].saved_width = (int)frame.w;
        _canvas_data[window_id].saved_height = (int)frame.h;
        _canvas_data[window_id].saved_style_mask = style_mask;

        msg_void_id(window, sel_c("toggleFullScreen:"), NULL);
        canvas_info.canvas[window_id].fullscreen = true;
    }

    canvas_info.canvas[window_id].minimized = false;
    canvas_info.canvas[window_id].maximized = false;

    CANVAS_RETURN(CANVAS_OK);
}

static objc_id _canvas_get_ns_cursor(canvas_cursor_type cursor)
{
    CANVAS_ENTER_FUNC();

    if (cursor < 0 || cursor >= sizeof(_cursor_selector_names) / sizeof(_cursor_selector_names[0]))
        cursor = CANVAS_CURSOR_ARROW;

    const char *selector = _cursor_selector_names[cursor];

    if (!selector)
        selector = "arrowCursor";

    objc_id result = msg_id(cls("NSCursor"), selector);
    CANVAS_RETURN(result);
}

int canvas_cursor(int window_id, canvas_cursor_type cursor)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    canvas_info.canvas[window_id].cursor = cursor;

    objc_id ns_cursor = _canvas_get_ns_cursor(cursor);
    if (ns_cursor)
        msg_void(ns_cursor, "set");

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_close(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    objc_id window = canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to close: %d\n", window_id);
    }

    if (_canvas_data[window_id].layer)
    {
        msg_void(_canvas_data[window_id].layer, "release");
    }

    _canvas_data[window_id].layer = NULL;
    _canvas_data[window_id].view = NULL;
    _canvas_data[window_id].scale = 0.0;

    msg_void(window, "close");
    msg_void(window, "release");

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_set(int window_id, int display, int64_t x, int64_t y, int64_t width, int64_t height, const char *title)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    objc_id window = canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to set: %d\n", window_id);
    }

    CANVAS_DISPLAY_BOUNDS(display);

    int global_x = canvas_info.display[display].x + x;
    int global_y = canvas_info.display[display].y + (canvas_info.display[display].height - (y + height));

    if (x >= 0 && y >= 0 && width > 0 && height > 0)
    {
        _CGRect rect = make_rect(global_x, global_y, width, height);
        msg_void_rect_bool(window, "setFrame:display:", rect, true);
    }

    if (title)
    {
        objc_id nsTitle = nsstring_from_cstr(title);
        if (nsTitle)
            msg_void_id(window, "setTitle:", nsTitle);
    }

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_get_window_display(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    objc_id window = canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to get display: %d\n", window_id);
    }

    objc_id screen = msg_id(window, "screen");
    if (!screen)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_DISPLAY, "no screen for window: %d\n", window_id);
    }

    objc_id device_desc = msg_id(screen, "deviceDescription");
    if (!device_desc)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_GPU, "no device description: %d\n", window_id);
    }

    objc_id screen_number_key = nsstring_from_cstr("NSScreenNumber");
    if (!screen_number_key)
    {
        CANVAS_RETURN(CANVAS_ERR_GET_DISPLAY);
    }

    objc_id display_id_obj = msg_id_id(device_desc, "objectForKey:", screen_number_key);
    if (!display_id_obj)
    {
        CANVAS_RETURN(CANVAS_ERR_GET_DISPLAY);
    }

    unsigned long display_id = msg_ulong(display_id_obj, "unsignedIntValue");

    uint32_t max_displays = MAX_DISPLAYS;
    CGDirectDisplayID displays[MAX_DISPLAYS];
    uint32_t display_count = 0;

    CGGetActiveDisplayList(max_displays, displays, &display_count);

    for (uint32_t i = 0; i < display_count && i < MAX_DISPLAYS; i++)
    {
        if (displays[i] != (CGDirectDisplayID)display_id)
            continue;

        canvas_info.canvas[window_id].display = i;
        CANVAS_RETURN(i);
    }

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_refresh_displays()
{
    CANVAS_ENTER_FUNC();

    canvas_info.display_count = 0;
    canvas_info.display_changed = false;
    canvas_info.highest_refresh_rate = 0;

    uint32_t max_displays = MAX_DISPLAYS;
    CGDirectDisplayID display[MAX_DISPLAYS];
    uint32_t display_count = 0;

    CGError err = CGGetActiveDisplayList(max_displays, display, &display_count);
    if (err != kCGErrorSuccess)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_DISPLAY, "get display list: %d\n", err);
    }

    CGDirectDisplayID main_display = CGMainDisplayID();

    for (uint32_t i = 0; i < display_count && i < MAX_DISPLAYS; i++)
    {
        CGDirectDisplayID display_id = display[i];
        CGRect bounds = CGDisplayBounds(display_id);
        CGDisplayModeRef mode = CGDisplayCopyDisplayMode(display_id);
        double refresh_rate = 60.0;

        if (mode)
        {
            refresh_rate = CGDisplayModeGetRefreshRate(mode);

            if (refresh_rate <= 0.0)
                refresh_rate = 60.0;

            CGDisplayModeRelease(mode);
        }

        canvas_info.display[i].primary = (display_id == main_display);
        canvas_info.display[i].x = (int)bounds.origin.x;
        canvas_info.display[i].y = (int)bounds.origin.y;
        canvas_info.display[i].width = (int)bounds.size.width;
        canvas_info.display[i].height = (int)bounds.size.height;
        canvas_info.display[i].refresh_rate = (int)refresh_rate;

        if ((int)refresh_rate > canvas_info.highest_refresh_rate)
            canvas_info.highest_refresh_rate = (int)refresh_rate;

        canvas_info.display_count++;
    }

    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (canvas_info.canvas[i].window)
            _canvas_get_window_display(i);
    }

    CANVAS_RETURN(canvas_info.display_count);
}

void _canvas_display_reconfigure_callback(CGDirectDisplayID display, CGDisplayChangeSummaryFlags flags, void *userInfo)
{
    CANVAS_ENTER_FUNC();

    if (flags & (kCGDisplayAddFlag | kCGDisplayRemoveFlag | kCGDisplaySetModeFlag | kCGDisplayDesktopShapeChangedFlag))
        canvas_info.display_changed = true;

    CANVAS_RETURN_VOID();
}

int _canvas_init_displays()
{
    CANVAS_ENTER_FUNC();

    canvas_info.display_count = 0;
    CGDisplayRegisterReconfigurationCallback(_canvas_display_reconfigure_callback, NULL);

    int result = _canvas_refresh_displays();
    CANVAS_RETURN(result);
}

void _post_init()
{
    CANVAS_ENTER_FUNC();

    if (canvas_info.init_post)
    {
        CANVAS_RETURN_VOID();
    }
    canvas_info.init_post = 1;

    objc_id poolClass = cls("NSAutoreleasePool");
    if (!poolClass)
    {
        CANVAS_RETURN_VOID();
    }

    objc_id alloc = msg_id(poolClass, "alloc");
    canvas_macos.pool = msg_id(alloc, "init");
    mach_timebase_info(&canvas_macos.timebase);

    CANVAS_RETURN_VOID();
}

static void hid_input_cb(void *ctx, IOReturn res, void *sender, IOHIDValueRef value)
{
    IOHIDElementRef elem = IOHIDValueGetElement(value);
    uint32_t page = IOHIDElementGetUsagePage(elem);
    uint32_t usage = IOHIDElementGetUsage(elem);

    if (page == 0x07) // USB HID Keyboard/Keypad page id
    {
        bool pressed = IOHIDValueGetIntegerValue(value) != 0;
        int hid_key = (int)usage;

        if (hid_key > 0 && hid_key < 256)
        {
            bool was_down = canvas_keyboard.keys[hid_key];
            canvas_keyboard.keys[hid_key] = pressed;

            if (pressed && !was_down)
            {
                canvas_keyboard.keys_pressed[hid_key] = true;
            }
            else if (!pressed && was_down)
            {
                canvas_keyboard.keys_released[hid_key] = true;
            }
        }
    }
}

static IOHIDManagerRef start_hid()
{
    IOHIDManagerRef m = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

    const uint32_t devPage = 0x01, devUsage = 0x06;
    CFMutableDictionaryRef match = CFDictionaryCreateMutable(NULL, 0,
                                                             &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFNumberRef nPage = CFNumberCreate(NULL, kCFNumberIntType, &devPage);
    CFNumberRef nUsage = CFNumberCreate(NULL, kCFNumberIntType, &devUsage);
    CFDictionarySetValue(match, CFSTR(kIOHIDDeviceUsagePageKey), nPage);
    CFDictionarySetValue(match, CFSTR(kIOHIDDeviceUsageKey), nUsage);
    CFRelease(nPage);
    CFRelease(nUsage);

    IOHIDManagerSetDeviceMatching(m, match);
    CFRelease(match);

    IOHIDManagerRegisterInputValueCallback(m, hid_input_cb, NULL);
    IOHIDManagerScheduleWithRunLoop(m, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);

    IOReturn r = IOHIDManagerOpen(m, kIOHIDOptionsTypeNone);
    if (r != kIOReturnSuccess)
    {
    }
    return m;
}

int _canvas_platform()
{
    CANVAS_ENTER_FUNC();

    _post_init();

    canvas_macos.hid_manager = start_hid();

    canvas_macos.app = msg_id(cls("NSApplication"), "sharedApplication");
    if (!canvas_macos.app)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_PLATFORM, "failed to get NSApplication\n");
    }

    msg_void_long(canvas_macos.app, "setActivationPolicy:", NSApplicationActivationPolicyRegular);
    msg_void_bool(canvas_macos.app, "activateIgnoringOtherApps:", true);

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_window(int64_t x, int64_t y, int64_t width, int64_t height, const char *title)
{
    CANVAS_ENTER_FUNC();

    _post_init();
    canvas_startup();

    unsigned long style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;

    objc_id winClass = cls("NSWindow");
    if (!winClass)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "failed to get NSWindow class\n");
    }

    int window_id = _canvas_get_free();
    if (window_id < 0)
    {
        CANVAS_RETURN(window_id);
    }

    objc_id walloc = msg_id(winClass, "alloc");

    typedef objc_id (*msg_initWin)(objc_id, objc_sel, _CGRect, unsigned long, long, int);

    _CGRect rect = make_rect(x, y, width, height);

    objc_id window = ((msg_initWin)objc_msgSend)(walloc, sel_c("initWithContentRect:styleMask:backing:defer:"), rect, style, (long)NSBackingStoreBuffered, 0);

    if (!window)
    {
        canvas_info.canvas[window_id]._valid = false;
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "failed to create NSWindow\n");
    }

    msg_void_bool(window, "setTitlebarAppearsTransparent:", true);
    msg_void_long(window, "setTitleVisibility:", NSWindowTitleHidden);

    unsigned long sm = msg_ulong(window, "styleMask");
    sm |= NSWindowStyleMaskFullSizeContent;
    msg_void_long(window, "setStyleMask:", sm);

    if (title)
    {
        objc_id nsTitle = nsstring_from_cstr(title);
        if (nsTitle)
            msg_void_id(window, "setTitle:", nsTitle);
    }

    msg_void_id(window, "makeKeyAndOrderFront:", NULL);

    canvas_info.canvas[window_id].window = window;
    canvas_info.canvas[window_id].resize = false;
    canvas_info.canvas[window_id].index = window_id;
    canvas_info.canvas[window_id].minimized = false;
    canvas_info.canvas[window_id].maximized = false;
    canvas_info.canvas[window_id].fullscreen = false;
    canvas_info.canvas[window_id]._valid = true;

    CANVAS_RETURN(window_id);
}

int _canvas_gpu_init()
{
    CANVAS_ENTER_FUNC();

    if (canvas_info.init_gpu)
    {
        CANVAS_RETURN(CANVAS_OK);
    }
    canvas_info.init_gpu = 1;

    canvas_macos.device = MTLCreateSystemDefaultDevice();
    if (!canvas_macos.device)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_GPU, "failed to create Metal device\n");
    }

    canvas_macos.queue = msg_id(canvas_macos.device, "newCommandQueue");
    if (!canvas_macos.queue)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_GPU, "failed to create Metal command queue\n");
    }

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_update_drawable_size(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    if (!_canvas_data[window_id].view || !_canvas_data[window_id].layer)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_DISPLAY, "no view or layer to update drawable size: %d\n", window_id);
    }

    _CGRect b = msg_rect(_canvas_data[window_id].view, "bounds");
    double w = b.w * _canvas_data[window_id].scale;
    double h = b.h * _canvas_data[window_id].scale;
    msg_void_size(_canvas_data[window_id].layer, "setDrawableSize:", w, h);

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_window_resize(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    if (!_canvas_data[window_id].layer)
    {
        CANVAS_RETURN(CANVAS_OK);
    }

    canvas_info.canvas[window_id].resize = false;

    _canvas_update_drawable_size(window_id);

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_gpu_new_window(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    objc_id window = canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window for GPU setup: %d\n", window_id);
    }

    objc_id content_view = msg_id(window, "contentView");
    if (!content_view)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no content view: %d\n", window_id);
    }

    msg_void_bool(content_view, "setWantsLayer:", true);

    objc_id layer_class = cls("CAMetalLayer");
    if (!layer_class)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_GPU, "failed to get CAMetalLayer class\n");
    }

    objc_id layer_alloc = msg_id(layer_class, "alloc");
    objc_id layer = msg_id(layer_alloc, "init");

    if (!layer)
    {
        msg_void(layer_alloc, "release");
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_GPU, "failed to create Metal layer\n");
    }

    msg_void_id(layer, "setDevice:", canvas_macos.device);
    msg_void_bool(layer, "setPresentsWithTransaction:", false);

    msg_void_id(content_view, "setLayer:", layer);

    /* backingScaleFactor returns a double (CGFloat) on NSWindow, call it directly */
    double scale = msg_dbl(window, "backingScaleFactor");

    if (scale == 0.0)
        scale = 1.0;

    msg_void_double(layer, "setContentsScale:", scale);

    _canvas_data[window_id].layer = layer;
    _canvas_data[window_id].view = content_view;
    _canvas_data[window_id].scale = scale;

    _canvas_update_drawable_size(window_id);

    CANVAS_VERBOSE("metal layer setup complete for window %d\n", window_id);

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_present(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    if (!_canvas_data[window_id].layer)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no layer to present: %d\n", window_id);
    }

    objc_id drawable = msg_id(_canvas_data[window_id].layer, "nextDrawable");
    if (!drawable)
    {
        CANVAS_WARN("no drawable available for window %d\n", window_id);
        CANVAS_RETURN(CANVAS_OK);
    }

    objc_id cmdBuffer = msg_id(canvas_macos.queue, "commandBuffer");
    if (!cmdBuffer)
    {
        CANVAS_RETURN_ERR(CANVAS_FAIL, "failed to create command buffer\n");
    }

    objc_id renderPassDescriptor = msg_id(cls("MTLRenderPassDescriptor"), "renderPassDescriptor");
    if (!renderPassDescriptor)
    {
        CANVAS_RETURN_ERR(CANVAS_FAIL, "failed to create render pass descriptor\n");
    }

    objc_id color_attachments = msg_id(renderPassDescriptor, "colorAttachments");
    objc_id color_attachment = msg_id_ulong(color_attachments, "objectAtIndexedSubscript:", 0);

    objc_id texture = msg_id(drawable, "texture");
    msg_void_id(color_attachment, "setTexture:", texture);
    msg_void_long(color_attachment, "setLoadAction:", MTLLoadActionClear);
    msg_void_long(color_attachment, "setStoreAction:", MTLStoreActionStore);

    _MTLClearColor clear_color = make_clear_color(
        canvas_info.canvas[window_id].clear[0],
        canvas_info.canvas[window_id].clear[1],
        canvas_info.canvas[window_id].clear[2],
        canvas_info.canvas[window_id].clear[3]);

    msg_void_clear_color(color_attachment, "setClearColor:", clear_color);

    objc_id renderEncoder = msg_id_id_descriptor(cmdBuffer, "renderCommandEncoderWithDescriptor:", renderPassDescriptor);
    if (!renderEncoder)
    {
        CANVAS_RETURN_ERR(CANVAS_FAIL, "failed to create render encoder\n");
    }

    msg_void(renderEncoder, "endEncoding");
    msg_void_id(cmdBuffer, "presentDrawable:", drawable);
    msg_void(cmdBuffer, "commit");

    CANVAS_RETURN(CANVAS_OK);
}

void _canvas_gpu_draw_all()
{
    CANVAS_ENTER_FUNC();

    if (!canvas_macos.queue)
    {
        CANVAS_VERBOSE("no metal command queue\n");
        CANVAS_RETURN_VOID();
    }

    for (int i = 0; i < MAX_CANVAS; ++i)
    {
        if (!canvas_info.canvas[i].window || !_canvas_data[i].layer)
            continue;

        _canvas_update_drawable_size(i);

        objc_id layer = _canvas_data[i].layer;
        if (!layer)
            continue;

        objc_id drawable = msg_id(layer, "nextDrawable");
        if (!drawable)
            continue;

        objc_id texture = msg_id(drawable, "texture");

        objc_id rpd = msg_id(cls("MTLRenderPassDescriptor"), "renderPassDescriptor");
        objc_id caps = msg_id(rpd, "colorAttachments");
        objc_id att0 = msg_id_ulong(caps, "objectAtIndexedSubscript:", 0UL);

        msg_void_id(att0, "setTexture:", texture);
        msg_void_long(att0, "setLoadAction:", MTLLoadActionClear);
        msg_void_long(att0, "setStoreAction:", MTLStoreActionStore);

        _MTLClearColor clear = make_clear_color(
            canvas_info.canvas[i].clear[0],
            canvas_info.canvas[i].clear[1],
            canvas_info.canvas[i].clear[2],
            canvas_info.canvas[i].clear[3]);
        msg_void_clear_color(att0, "setClearColor:", clear);

        objc_id cmd = msg_id(canvas_macos.queue, "commandBuffer");
        objc_id enc = msg_id_id_descriptor(cmd, "renderCommandEncoderWithDescriptor:", rpd);
        msg_void(enc, "endEncoding");

        msg_void_id(cmd, "presentDrawable:", drawable);
        msg_void(cmd, "commit");
    }

    CANVAS_RETURN_VOID();
}

static int _metal_upload_buffer_data(canvas_buffer *buf, void *data, size_t size)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT(buf != NULL);
    CANVAS_ASSERT(data != NULL);
    CANVAS_ASSERT(size > 0);

    typedef objc_id (*msg_new_buffer)(objc_id, objc_sel, unsigned long, unsigned long);
    objc_id staging = ((msg_new_buffer)objc_msgSend)(
        canvas_macos.device,
        sel_c("newBufferWithLength:options:"),
        (unsigned long)size,
        0 // MTLResourceStorageModeShared
    );

    if (!staging)
    {
        CANVAS_RETURN_ERR(CANVAS_FAIL, "Failed to create staging buffer\n");
    }

    void *staging_ptr = msg_id(staging, "contents");
    memcpy(staging_ptr, data, size);

    objc_id cmd_buffer = msg_id(canvas_macos.queue, "commandBuffer");
    if (!cmd_buffer)
    {
        msg_void(staging, "release");
        CANVAS_RETURN_ERR(CANVAS_FAIL, "Failed to create command buffer\n");
    }

    objc_id blit_encoder = msg_id(cmd_buffer, "blitCommandEncoder");
    if (!blit_encoder)
    {
        msg_void(staging, "release");
        CANVAS_RETURN_ERR(CANVAS_FAIL, "Failed to create blit encoder\n");
    }

    typedef void (*msg_copy_buffer)(objc_id, objc_sel, objc_id, unsigned long, objc_id, unsigned long, unsigned long);
    ((msg_copy_buffer)objc_msgSend)(
        blit_encoder,
        sel_c("copyFromBuffer:sourceOffset:toBuffer:destinationOffset:size:"),
        staging,
        0UL,
        buf->platform_handle,
        0UL,
        (unsigned long)size);

    msg_void(blit_encoder, "endEncoding");

    msg_void(cmd_buffer, "commit");
    msg_void(cmd_buffer, "waitUntilCompleted");

    msg_void(staging, "release");

    CANVAS_RETURN(CANVAS_OK);
}

canvas_buffer *canvas_buffer_create(int window_id, canvas_buffer_type type, canvas_buffer_usage usage, size_t size, void *initial_data)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);
    CANVAS_ASSERT(size > 0);

    CANVAS_VALID_PTR(window_id);

    if (!canvas_macos.device)
    {
        CANVAS_RETURN_ERR(NULL, "GPU not initialized\n");
    }

    canvas_buffer *buf = (canvas_buffer *)calloc(1, sizeof(canvas_buffer));
    if (!buf)
    {
        CANVAS_RETURN(NULL);
    }

    buf->size = size;
    buf->type = type;
    buf->usage = usage;
    buf->window_id = window_id;

    unsigned long storage_mode;

    if (usage == CANVAS_BUFFER_DYNAMIC)
    {
        storage_mode = 0; // MTLResourceStorageModeShared
    }
    else
    {
        storage_mode = 2; // MTLResourceStorageModePrivate
    }

    typedef objc_id (*msg_new_buffer)(objc_id, objc_sel, unsigned long, unsigned long);
    objc_id metal_buffer = ((msg_new_buffer)objc_msgSend)(
        canvas_macos.device,
        sel_c("newBufferWithLength:options:"),
        (unsigned long)size,
        storage_mode);

    if (!metal_buffer)
    {
        CANVAS_ERR("Failed to create Metal buffer\n");
        free(buf);
        CANVAS_RETURN(NULL);
    }

    buf->platform_handle = metal_buffer;

    if (usage == CANVAS_BUFFER_DYNAMIC)
    {
        buf->mapped = msg_id(metal_buffer, "contents");
        if (initial_data)
        {
            memcpy(buf->mapped, initial_data, size);
        }
    }
    else if (initial_data)
    {
        if (_metal_upload_buffer_data(buf, initial_data, size) != CANVAS_OK)
        {
            CANVAS_ERR("Failed to upload private buffer data\n");
            msg_void(metal_buffer, "release");
            free(buf);
            CANVAS_RETURN(NULL);
        }
    }

    CANVAS_RETURN(buf);
}

void canvas_buffer_update(canvas_buffer *buf, void *data, size_t size, size_t offset)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(buf);
    CANVAS_ASSERT_NOT_NULL(data);
    CANVAS_ASSERT(size > 0);

    if (!buf || !data)
    {
        CANVAS_RETURN_VOID();
    }

    if (buf->usage != CANVAS_BUFFER_DYNAMIC)
    {
        CANVAS_WARN("Can only update dynamic buffers\n");
        CANVAS_RETURN_VOID();
    }

    if (offset > buf->size || size > buf->size - offset)
    {
        CANVAS_ERR("Buffer update out of bounds\n");
        CANVAS_RETURN_VOID();
    }

    memcpy((uint8_t *)buf->mapped + offset, data, size);

    // Synchronize shared memory (if needed on older hardware)
    typedef void (*msg_did_modify)(objc_id, objc_sel, unsigned long, unsigned long);
    ((msg_did_modify)objc_msgSend)(
        buf->platform_handle,
        sel_c("didModifyRange:"),
        offset,
        size);
    CANVAS_RETURN_VOID();
}

void *canvas_buffer_map(canvas_buffer *buf)
{
    CANVAS_ENTER_FUNC();

    if (!buf)
    {
        CANVAS_RETURN(NULL);
    }

    if (buf->usage == CANVAS_BUFFER_DYNAMIC)
    {
        CANVAS_RETURN(buf->mapped); // Already mapped
    }

    CANVAS_WARN("Cannot map private Metal buffer\n");
    CANVAS_RETURN(NULL);
}

void canvas_buffer_unmap(canvas_buffer *buf)
{
    CANVAS_ENTER_FUNC();
    // Metal shared buffers are persistently mapped
    (void)buf;
    CANVAS_RETURN_VOID();
}

void canvas_buffer_destroy(canvas_buffer *buf)
{
    CANVAS_ENTER_FUNC();

    if (!buf)
    {
        CANVAS_RETURN_VOID();
    }

    objc_id metal_buffer = (objc_id)buf->platform_handle;
    msg_void(metal_buffer, "release");

    free(buf);
    CANVAS_RETURN_VOID();
}

int _canvas_update()
{
    CANVAS_ENTER_FUNC();

    _post_init();

    if (canvas_info.display_changed)
        _canvas_refresh_displays();

    objc_id poolClass = cls("NSAutoreleasePool");
    objc_id framePool = NULL;
    if (poolClass)
    {
        objc_id tmp = msg_id(poolClass, "alloc");
        framePool = msg_id(tmp, "init");
    }

    canvas_pointer *p = canvas_get_primary_pointer(0);

    objc_id ns_event_class = cls("NSEvent");
    typedef _CGRect (*msg_mouse_location)(objc_id, objc_sel);
    _CGRect mouse_loc = ((msg_mouse_location)objc_msgSend)(ns_event_class, sel_c("mouseLocation"));

    int global_x = (int)mouse_loc.x;
    int global_y = (int)mouse_loc.y;

    p->display = 0;
    for (int d = 0; d < canvas_info.display_count; d++)
    {
        if (global_x >= canvas_info.display[d].x &&
            global_x < canvas_info.display[d].x + canvas_info.display[d].width &&
            global_y >= canvas_info.display[d].y &&
            global_y < canvas_info.display[d].y + canvas_info.display[d].height)
        {
            p->display = d;
            p->screen_x = global_x - canvas_info.display[d].x;
            p->screen_y = canvas_info.display[d].height - (global_y - canvas_info.display[d].y);
            break;
        }
    }

    uint64_t now = mach_absolute_time();
    double timestamp = (double)now * (double)canvas_macos.timebase.numer /
                       (double)canvas_macos.timebase.denom / 1e9;

    p->_samples[p->_sample_index].x = p->screen_x;
    p->_samples[p->_sample_index].y = p->screen_y;
    p->_samples[p->_sample_index].time = timestamp;
    p->_sample_index = (p->_sample_index + 1) % CANVAS_POINTER_SAMPLE_FRAMES;

    int active_window = -1;
    bool found_window = false;

    for (int i = MAX_CANVAS - 1; i >= 0; i--)
    {
        if (!canvas_info.canvas[i]._valid || !canvas_info.canvas[i].window)
            continue;

        objc_id window = canvas_info.canvas[i].window;
        _CGRect frame = msg_rect(window, "frame");

        int win_x = (int)frame.x;
        int win_y = (int)frame.y;
        int win_w = (int)frame.w;
        int win_h = (int)frame.h;

        bool is_inside = (global_x >= win_x && global_x < win_x + win_w &&
                          global_y >= win_y && global_y < win_y + win_h);

        if (is_inside && !found_window)
        {
            active_window = i;
            found_window = true;

            p->window_id = i;
            p->inside_window = true;
            p->x = global_x - win_x;
            p->y = win_h - (global_y - win_y);

            objc_id ns_cursor = _canvas_get_ns_cursor(canvas_info.canvas[i].cursor);
            if (ns_cursor)
                msg_void(ns_cursor, "set");

            break;
        }
    }

    if (!found_window)
    {
        p->inside_window = false;
        p->x = 0;
        p->y = 0;
    }

    objc_id ns_mode = nsstring_from_cstr("kCFRunLoopDefaultMode");
    objc_id distantPast = msg_id(cls("NSDate"), "distantPast");

    for (;;)
    {
        objc_id ev = canvas_next_event(canvas_macos.app, ~0ULL, distantPast, ns_mode, true);
        if (!ev)
            break;

        unsigned long eventType = msg_ulong(ev, "type");
        objc_id eventWindow = msg_id(ev, "window");

        if (eventWindow)
        {
            int window_idx = _canvas_window_index(eventWindow);
            if (window_idx >= 0)
            {
                switch (eventType)
                {
                case 1: // NSEventTypeLeftMouseDown
                {
                    canvas_pointer *p = canvas_get_primary_pointer(window_idx);
                    p->buttons |= CANVAS_BUTTON_LEFT;
                    p->buttons_pressed |= CANVAS_BUTTON_LEFT;
                    p->window_id = window_idx;
                    break;
                }

                case 2: // NSEventTypeLeftMouseUp
                {
                    canvas_pointer *p = canvas_get_primary_pointer(window_idx);
                    p->buttons &= ~CANVAS_BUTTON_LEFT;
                    p->buttons_released |= CANVAS_BUTTON_LEFT;
                    break;
                }

                case 3: // NSEventTypeRightMouseDown
                {
                    canvas_pointer *p = canvas_get_primary_pointer(window_idx);
                    p->buttons |= CANVAS_BUTTON_RIGHT;
                    p->buttons_pressed |= CANVAS_BUTTON_RIGHT;
                    p->window_id = window_idx;
                    break;
                }

                case 4: // NSEventTypeRightMouseUp
                {
                    canvas_pointer *p = canvas_get_primary_pointer(window_idx);
                    p->buttons &= ~CANVAS_BUTTON_RIGHT;
                    p->buttons_released |= CANVAS_BUTTON_RIGHT;
                    break;
                }

                case 25: // NSEventTypeOtherMouseDown
                {
                    canvas_pointer *p = canvas_get_primary_pointer(window_idx);
                    long button_number = msg_ulong(ev, "buttonNumber");
                    if (button_number == 2)
                    {
                        p->buttons |= CANVAS_BUTTON_MIDDLE;
                        p->buttons_pressed |= CANVAS_BUTTON_MIDDLE;
                    }
                    else if (button_number == 3)
                    {
                        p->buttons |= CANVAS_BUTTON_X1;
                        p->buttons_pressed |= CANVAS_BUTTON_X1;
                    }
                    else if (button_number == 4)
                    {
                        p->buttons |= CANVAS_BUTTON_X2;
                        p->buttons_pressed |= CANVAS_BUTTON_X2;
                    }
                    p->window_id = window_idx;
                    break;
                }

                case 26: // NSEventTypeOtherMouseUp
                {
                    canvas_pointer *p = canvas_get_primary_pointer(window_idx);
                    long button_number = msg_ulong(ev, "buttonNumber");
                    if (button_number == 2)
                    {
                        p->buttons &= ~CANVAS_BUTTON_MIDDLE;
                        p->buttons_released |= CANVAS_BUTTON_MIDDLE;
                    }
                    else if (button_number == 3)
                    {
                        p->buttons &= ~CANVAS_BUTTON_X1;
                        p->buttons_released |= CANVAS_BUTTON_X1;
                    }
                    else if (button_number == 4)
                    {
                        p->buttons &= ~CANVAS_BUTTON_X2;
                        p->buttons_released |= CANVAS_BUTTON_X2;
                    }
                    break;
                }

                case 22: // NSEventTypeScrollWheel
                {
                    canvas_pointer *p = canvas_get_primary_pointer(window_idx);
                    p->scroll_y = (float)msg_dbl(ev, "scrollingDeltaY");
                    p->scroll_x = (float)msg_dbl(ev, "scrollingDeltaX");
                    break;
                }

                case 10: // NSEventTypeKeyDown
                case 11: // NSEventTypeKeyUp
                case 12: // NSEventTypeFlagsChanged
                    break;

                case 6:  // NSEventTypeLeftMouseDragged
                case 7:  // NSEventTypeRightMouseDragged
                case 27: // NSEventTypeOtherMouseDragged
                {
                    _CGRect frame = msg_rect(eventWindow, "frame");

                    if ((int)frame.x != canvas_info.canvas[window_idx].x ||
                        (int)frame.y != canvas_info.canvas[window_idx].y)
                    {
                        canvas_info.canvas[window_idx].os_moved = true;
                        canvas_info.canvas[window_idx].x = (int)frame.x;
                        canvas_info.canvas[window_idx].y = (int)frame.y;
                    }

                    if ((int)frame.w != canvas_info.canvas[window_idx].width ||
                        (int)frame.h != canvas_info.canvas[window_idx].height)
                    {
                        canvas_info.canvas[window_idx].resize = true;
                        canvas_info.canvas[window_idx].os_resized = true;
                        canvas_info.canvas[window_idx].width = (int)frame.w;
                        canvas_info.canvas[window_idx].height = (int)frame.h;
                    }
                    break;
                }
                }
            }
        }

        msg_void_id(canvas_macos.app, "sendEvent:", ev);
    }

    objc_id tracking_mode = nsstring_from_cstr("NSEventTrackingRunLoopMode");
    for (;;)
    {
        objc_id ev = canvas_next_event(canvas_macos.app, ~0ULL, distantPast, tracking_mode, true);
        if (!ev)
            break;
        msg_void_id(canvas_macos.app, "sendEvent:", ev);
    }

    msg_void(canvas_macos.app, "updateWindows");
    _canvas_gpu_draw_all();

    if (framePool)
        msg_void(framePool, "drain");

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_post_update()
{
    CANVAS_ENTER_FUNC();
    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_exit()
{
    CANVAS_ENTER_FUNC();

    if (canvas_macos.queue)
    {
        ((MSG_void_id)objc_msgSend)(canvas_macos.queue, sel_c("release"));
        canvas_macos.queue = NULL;
    }
    if (canvas_macos.device)
    {
        ((MSG_void_id)objc_msgSend)(canvas_macos.device, sel_c("release"));
        canvas_macos.device = NULL;
    }
    if (canvas_macos.pool)
    {
        ((MSG_void_id)objc_msgSend)(canvas_macos.pool, sel_c("drain"));
        canvas_macos.pool = NULL;
    }

    if (canvas_macos.hid_manager)
    {
        IOHIDManagerClose(canvas_macos.hid_manager, kIOHIDOptionsTypeNone);
        CFRelease(canvas_macos.hid_manager);
        canvas_macos.hid_manager = NULL;
    }

    CGDisplayRemoveReconfigurationCallback(_canvas_display_reconfigure_callback, NULL);
    CANVAS_RETURN(CANVAS_OK);
}

static inline void _canvas_ensure_timebase()
{
    if (canvas_macos.timebase.denom == 0)
        mach_timebase_info(&canvas_macos.timebase);
}

double canvas_get_time(canvas_time_data *time)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT(time != NULL);

    _canvas_ensure_timebase();
    uint64_t elapsed = mach_absolute_time() - time->start;
    double result = (double)elapsed * (double)canvas_macos.timebase.numer /
                    (double)canvas_macos.timebase.denom / 1e9;
    CANVAS_RETURN(result);
}

void canvas_sleep(double seconds)
{
    CANVAS_ENTER_FUNC();

    if (seconds <= 0.0)
    {
        CANVAS_RETURN_VOID();
    }

    struct timespec ts;
    ts.tv_sec = (time_t)seconds;
    double frac = seconds - (double)ts.tv_sec;

    if (frac < 0.0)
        frac = 0.0;
    ts.tv_nsec = (long)(frac * 1e9);
    if (ts.tv_nsec >= 1000000000L)
    {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000L;
    }

    nanosleep(&ts, NULL);
    CANVAS_RETURN_VOID();
}

void canvas_time_init(canvas_time_data *time)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT(time != NULL);

    _canvas_ensure_timebase();

    time->frame = 0;
    time->frame_index = 0;
    time->accumulator = 0;
    time->alpha = 0;
    time->delta = 0;
    time->fps = 0;
    time->raw_delta = 0;

    for (int i = 0; i < 60; i++)
        time->times[i] = 0.0;

    time->start = mach_absolute_time();
    time->current = canvas_get_time(time);
    time->last = time->current;
    CANVAS_RETURN_VOID();
}

#endif /* __APPLE__ */

//
//
//
#if defined(_WIN32) || defined(_WIN64)

int canvas_minimize(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    HWND window = (HWND)canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to minimize: %d\n", window_id);
    }

    ShowWindow(window, SW_MINIMIZE);
    canvas_info.canvas[window_id].minimized = true;
    canvas_info.canvas[window_id].maximized = false;

    CANVAS_RETURN(CANVAS_OK);
}

int canvas_maximize(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    HWND window = (HWND)canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to maximize: %d\n", window_id);
    }

    ShowWindow(window, SW_MAXIMIZE);
    canvas_info.canvas[window_id].maximized = true;
    canvas_info.canvas[window_id].minimized = false;

    CANVAS_RETURN(CANVAS_OK);
}

int canvas_restore(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    HWND window = (HWND)canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to restore: %d\n", window_id);
    }

    if (canvas_info.canvas[window_id].fullscreen)
    {
        SetWindowLong(window, GWL_STYLE, _canvas_data[window_id].saved_style);

        HWND insertAfter = _canvas_data[window_id].saved_z_order ? _canvas_data[window_id].saved_z_order : HWND_NOTOPMOST;
        SetWindowPos(window, insertAfter,
                     (int)_canvas_data[window_id].saved_x,
                     (int)_canvas_data[window_id].saved_y,
                     (unsigned int)_canvas_data[window_id].saved_width,
                     (unsigned int)_canvas_data[window_id].saved_height,
                     SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOACTIVATE);

        ShowWindow(window, SW_NORMAL);
    }
    else
    {
        ShowWindow(window, SW_RESTORE);
    }

    canvas_info.canvas[window_id].minimized = false;
    canvas_info.canvas[window_id].maximized = false;
    canvas_info.canvas[window_id].fullscreen = false;

    CANVAS_RETURN(CANVAS_OK);
}

int canvas_fullscreen(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    HWND window = (HWND)canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window for fullscreen: %d\n", window_id);
    }

    if (!canvas_info.canvas[window_id].fullscreen)
    {
        RECT rect;
        GetWindowRect(window, &rect);
        _canvas_data[window_id].saved_x = rect.left;
        _canvas_data[window_id].saved_y = rect.top;
        _canvas_data[window_id].saved_width = rect.right - rect.left;
        _canvas_data[window_id].saved_height = rect.bottom - rect.top;
        _canvas_data[window_id].saved_style = GetWindowLong(window, GWL_STYLE);
        _canvas_data[window_id].saved_z_order = GetWindow(window, GW_HWNDPREV);

        HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = {sizeof(mi)};
        GetMonitorInfo(monitor, &mi);

        SetWindowLong(window, GWL_STYLE, WS_VISIBLE | WS_POPUP);
        SetWindowPos(window, HWND_TOPMOST,
                     mi.rcMonitor.left,
                     mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOACTIVATE);

        ShowWindow(window, SW_MAXIMIZE);
        UpdateWindow(window);

        canvas_info.canvas[window_id].resize = true;
        _canvas_window_resize(window_id);

        canvas_info.canvas[window_id].fullscreen = true;
    }

    canvas_info.canvas[window_id].minimized = false;
    canvas_info.canvas[window_id].maximized = false;

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_close(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    if (canvas_win32.device && canvas_win32.fence && canvas_win32.cmdQueue)
    {
        canvas_win32.fence_value++;
        canvas_win32.cmdQueue->lpVtbl->Signal(canvas_win32.cmdQueue, canvas_win32.fence, canvas_win32.fence_value);
        if (canvas_win32.fence->lpVtbl->GetCompletedValue(canvas_win32.fence) < canvas_win32.fence_value)
        {
            canvas_win32.fence->lpVtbl->SetEventOnCompletion(canvas_win32.fence, canvas_win32.fence_value, canvas_win32.fence_event);
            WaitForSingleObject(canvas_win32.fence_event, INFINITE);
        }
    }

    if (canvas_info.canvas[window_id].depth_texture)
    {
        ID3D12Resource *depth = (ID3D12Resource *)canvas_info.canvas[window_id].depth_texture;
        depth->lpVtbl->Release(depth);
        canvas_info.canvas[window_id].depth_texture = NULL;
    }

    if (_canvas_data[window_id].swapChain)
    {
        _canvas_data[window_id].swapChain->lpVtbl->Release(_canvas_data[window_id].swapChain);
        _canvas_data[window_id].swapChain = NULL;
    }

    for (int i = 0; i < 2; i++)
    {
        if (_canvas_data[window_id].backBuffers[i])
        {
            _canvas_data[window_id].backBuffers[i]->lpVtbl->Release(_canvas_data[window_id].backBuffers[i]);
            _canvas_data[window_id].backBuffers[i] = NULL;
        }
    }

    DestroyWindow((HWND)canvas_info.canvas[window_id].window);
    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_set(int window_id, int display, int64_t x, int64_t y, int64_t width, int64_t height, const char *title)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    HWND window = (HWND)canvas_info.canvas[window_id].window;

    if (!window)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to set: %d\n", window_id);
    }

    if (display < 0 || display >= canvas_info.display_count)
        display = 0;

    CANVAS_DISPLAY_BOUNDS(display);

    int screen_x = canvas_info.display[display].x + x;
    int screen_y = canvas_info.display[display].y + y;

    SetWindowPos(window, NULL, screen_x, screen_y, width, height, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE);

    if (title)
        SetWindowTextA(window, title);

    canvas_info.canvas[window_id].display = display;

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_get_window_display(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    HWND window = (HWND)canvas_info.canvas[window_id].window;

    if (!window)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to set: %d\n", window_id);
    }

    HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY);

    for (int i = 0; i < canvas_info.display_count; i++)
    {
        DISPLAY_DEVICEA dd = {0};
        dd.cb = sizeof(dd);

        if (!EnumDisplayDevicesA(NULL, i, &dd, 0))
            continue;

        if (!(dd.StateFlags & DISPLAY_DEVICE_ACTIVE))
            continue;

        DEVMODEA dm = {0};
        dm.dmSize = sizeof(dm);

        if (!EnumDisplaySettingsA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm))
            continue;

        MONITORINFOEXA mi = {0};
        mi.cbSize = sizeof(mi);

        if (!GetMonitorInfoA(monitor, (MONITORINFO *)&mi))
            continue;

        if (strcmp(mi.szDevice, dd.DeviceName) != 0)
            continue;

        canvas_info.canvas[window_id].display = i;
        CANVAS_RETURN(i);
    }

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_refresh_displays()
{
    CANVAS_ENTER_FUNC();

    canvas_info.display_count = 0;
    canvas_info.display_changed = false;
    canvas_info.highest_refresh_rate = 0;

    for (int i = 0; i < MAX_DISPLAYS; i++)
    {
        DISPLAY_DEVICEA dd = {0};
        dd.cb = sizeof(dd);

        if (!EnumDisplayDevicesA(NULL, i, &dd, 0))
            continue;

        if (!(dd.StateFlags & DISPLAY_DEVICE_ACTIVE))
            continue;

        DEVMODEA dm = {0};
        dm.dmSize = sizeof(dm);

        if (!EnumDisplaySettingsA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm))
            continue;

        POINT pt = {dm.dmPosition.x + 1, dm.dmPosition.y + 1};
        HMONITOR monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

        canvas_win32.monitors[i] = monitor;

        canvas_info.display[i].primary = (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) != 0;
        canvas_info.display[i].x = dm.dmPosition.x;
        canvas_info.display[i].y = dm.dmPosition.y;
        canvas_info.display[i].width = dm.dmPelsWidth;
        canvas_info.display[i].height = dm.dmPelsHeight;
        canvas_info.display[i].refresh_rate = dm.dmDisplayFrequency;

        if (dm.dmDisplayFrequency > canvas_info.highest_refresh_rate)
            canvas_info.highest_refresh_rate = dm.dmDisplayFrequency;

        canvas_info.display_count++;
    }

    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (canvas_info.canvas[i].window)
            _canvas_get_window_display(i);
    }

    CANVAS_RETURN(canvas_info.display_count);
}
int _canvas_init_displays()
{
    CANVAS_ENTER_FUNC();

    canvas_info.display_count = 0;

    // SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    int result = _canvas_refresh_displays();
    CANVAS_RETURN(result);
}

double canvas_get_time(canvas_time_data *time)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT(time != NULL);

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    double result = (double)((uint64_t)counter.QuadPart - time->start) /
                    (double)_canvas_qpc_frequency.QuadPart;
    CANVAS_RETURN(result);
}

void canvas_sleep(double seconds)
{
    CANVAS_ENTER_FUNC();

    HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (timer)
    {
        LARGE_INTEGER li;
        li.QuadPart = -(LONGLONG)(seconds * 10000000.0); // 100ns units
        SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
    }

    CANVAS_RETURN_VOID();
}

void canvas_time_init(canvas_time_data *time)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT(time != NULL);

    time->frame = 0;
    time->frame_index = 0;
    time->accumulator = 0;
    time->alpha = 0;
    time->delta = 0;
    time->fps = 0;
    time->raw_delta = 0;

    for (int i = 0; i < 60; i++)
        time->times[i] = 0.0;

    QueryPerformanceFrequency(&_canvas_qpc_frequency);

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    time->start = (uint64_t)counter.QuadPart;
    time->current = canvas_get_time(time);
    time->last = time->current;
    CANVAS_RETURN_VOID();
}

HCURSOR _canvas_get_win32_cursor(canvas_cursor_type cursor)
{
    CANVAS_ENTER_FUNC();

    LPCTSTR cursor_name;

    switch (cursor)
    {
    case CANVAS_CURSOR_ARROW:
        cursor_name = IDC_ARROW;
        break;
    case CANVAS_CURSOR_TEXT:
        cursor_name = IDC_IBEAM;
        break;
    case CANVAS_CURSOR_CROSSHAIR:
        cursor_name = IDC_CROSS;
        break;
    case CANVAS_CURSOR_HAND:
        cursor_name = IDC_HAND;
        break;
    case CANVAS_CURSOR_SIZE_NS:
        cursor_name = IDC_SIZENS;
        break;
    case CANVAS_CURSOR_SIZE_EW:
        cursor_name = IDC_SIZEWE;
        break;
    case CANVAS_CURSOR_SIZE_NESW:
        cursor_name = IDC_SIZENESW;
        break;
    case CANVAS_CURSOR_SIZE_NWSE:
        cursor_name = IDC_SIZENWSE;
        break;
    case CANVAS_CURSOR_SIZE_ALL:
        cursor_name = IDC_SIZEALL;
        break;
    case CANVAS_CURSOR_NOT_ALLOWED:
        cursor_name = IDC_NO;
        break;
    case CANVAS_CURSOR_WAIT:
        cursor_name = IDC_WAIT;
        break;
    default:
        cursor_name = IDC_ARROW;
        break;
    }

    HCURSOR result = LoadCursor(NULL, cursor_name);
    CANVAS_RETURN(result);
}

int canvas_cursor(int window_id, canvas_cursor_type cursor)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    canvas_info.canvas[window_id].cursor = cursor;
    SetCursor(_canvas_get_win32_cursor(cursor));

    CANVAS_RETURN(CANVAS_OK);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int window_index = _canvas_window_index(hwnd);

    if (window_index < 0)
        return DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_MOUSEWHEEL:
    {
        canvas_pointer *p = canvas_get_primary_pointer(window_index);
        short delta = GET_WHEEL_DELTA_WPARAM(wParam);
        p->scroll_y = (float)delta / 120.0f;
        return 0;
    }

    case WM_MOUSEHWHEEL:
    {
        canvas_pointer *p = canvas_get_primary_pointer(window_index);
        short delta = GET_WHEEL_DELTA_WPARAM(wParam);
        p->scroll_x = (float)delta / 120.0f;
        return 0;
    }

    case WM_SETCURSOR:
    {
        if (LOWORD(lParam) == HTCLIENT)
        {
            SetCursor(_canvas_get_win32_cursor(canvas_info.canvas[window_index].cursor));
            return TRUE;
        }
        break;
    }

    case WM_CREATE:
    {
        if (!canvas_info.canvas[window_index].titlebar)
        {
            DWM_WINDOW_CORNER_PREFERENCE corner = DWMWCP_ROUND;
            DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));

            enum DWM_SYSTEMBACKDROP_TYPE backdrop = DWMSBT_MAINWINDOW;
            DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));
        }
        return CANVAS_OK;
    }

    case WM_NCCALCSIZE:
    {
        if (canvas_info.canvas[window_index].fullscreen)
            break;

        if (wParam == true && !canvas_info.canvas[window_index].titlebar)
        {
            NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS *)lParam;

            params->rgrc[0].top += 1;
            params->rgrc[0].right -= 8;
            params->rgrc[0].bottom -= 8;
            params->rgrc[0].left += 8;
            return 0;
        }
        break;
    }

    case WM_NCHITTEST:
    {
        if (canvas_info.canvas[window_index].fullscreen)
            break;

        if (!canvas_info.canvas[window_index].titlebar)
        {
            LRESULT hit = DefWindowProc(hwnd, msg, wParam, lParam);

            if (hit == HTCLIENT)
            {
                POINT pt = {LOWORD(lParam), HIWORD(lParam)};
                ScreenToClient(hwnd, &pt);

                RECT rcWindow;
                GetClientRect(hwnd, &rcWindow);

                if (pt.y < 30 && pt.y >= 0)
                {
                    return HTCAPTION;
                }
            }

            return hit;
        }

        break;
    }

    case WM_DISPLAYCHANGE:
    {
        canvas_info.canvas[window_index].os_moved = true;
        canvas_info.display_changed = true;
        return CANVAS_OK;
    }

    case WM_MOVE:
    {
        // _canvas_update_window_display(window_index);

        return CANVAS_OK;
    }

    case WM_MOVING:
    {
        canvas_info.canvas[window_index].os_moved = true;

        return CANVAS_OK;
    }

    case WM_SIZE:
    {

        if (wParam != SIZE_MINIMIZED)
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            if (width > 0 && height > 0)
            {
                canvas_info.canvas[window_index].width = width;
                canvas_info.canvas[window_index].height = height;
                canvas_info.canvas[window_index].resize = true;
                canvas_info.canvas[window_index].os_resized = true;
            }

            if (!canvas_info.os_timed)
            {
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        return CANVAS_OK;
    }

    case WM_SIZING:
    {

        canvas_info.canvas[window_index].os_resized = true;

        /* TODO
            RECT *rect = (RECT *)lParam;
            UINT width = rect->right - rect->left;
            UINT height = rect->bottom - rect->top;

            RECT client;
            GetClientRect(hwnd, &client);
            width = client.right - client.left;
            height = client.bottom - client.top;

            if (width > 0 && height > 0)
            {
                canvas_info.canvas[window_index].resize = true;
                _canvas_window_resize(window_index);
                _canvas_update();
            }
                */

        return TRUE;
    }

    case WM_ENTERSIZEMOVE:
    {
        if (canvas_info.os_timed)
            break;

        canvas_info.os_timed = true;
        timeBeginPeriod(1);
        SetTimer(hwnd, 1, 0, NULL);
        return CANVAS_OK;
    }

    case WM_EXITSIZEMOVE:
    {
        if (!canvas_info.os_timed)
            break;

        canvas_info.os_timed = false;
        KillTimer(hwnd, 1);
        timeEndPeriod(1);
        return CANVAS_OK;
    }

    case WM_TIMER:
    {
        if (wParam != 1)
            break;

        canvas_main_loop();

        return CANVAS_OK;
    }

    case WM_SYSCOMMAND:
    {
        if (wParam == SC_MINIMIZE)
            canvas_info.canvas[window_index].minimized = true;
        else if (wParam == SC_MAXIMIZE)
            canvas_info.canvas[window_index].maximized = true;
        else if (wParam == SC_RESTORE)
        {
            canvas_info.canvas[window_index].minimized = false;
            canvas_info.canvas[window_index].maximized = false;
        }
        break;
    }

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        int scancode = (lParam >> 16) & 0xFF;
        bool extended = (lParam >> 24) & 1;

        if (extended)
        {
            if (scancode == 0x1D)
                scancode = 0xE01D;
            else if (scancode == 0x38)
                scancode = 0xE038;
            else
                scancode |= 0xE000;
        }

        int hid = 0;
        if (scancode < 128)
            hid = win32_to_hid[scancode];
        else if (scancode == 0xE01D)
            hid = 228;
        else if (scancode == 0xE038)
            hid = 230;
        else if (scancode == 0xE05B)
            hid = 227;
        else if (scancode == 0xE05C)
            hid = 231;

        if (hid > 0 && hid < 256 && !canvas_keyboard.keys[hid])
        {
            canvas_keyboard.keys[hid] = true;
            canvas_keyboard.keys_pressed[hid] = true;
        }
        return 0;
    }

    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        int scancode = (lParam >> 16) & 0xFF;
        bool extended = (lParam >> 24) & 1;

        if (extended)
        {
            if (scancode == 0x1D)
                scancode = 0xE01D;
            else if (scancode == 0x38)
                scancode = 0xE038;
            else
                scancode |= 0xE000;
        }

        int hid = 0;
        if (scancode < 128)
            hid = win32_to_hid[scancode];
        else if (scancode == 0xE01D)
            hid = 228;
        else if (scancode == 0xE038)
            hid = 230;
        else if (scancode == 0xE05B)
            hid = 227;
        else if (scancode == 0xE05C)
            hid = 231;

        if (hid > 0 && hid < 256 && canvas_keyboard.keys[hid])
        {
            canvas_keyboard.keys[hid] = false;
            canvas_keyboard.keys_released[hid] = true;
        }
        return 0;
    }

    case WM_CLOSE:
    {
        canvas_info.canvas[window_index].close = true;

        if (canvas_info.os_timed)
        {
            KillTimer(hwnd, 1);
            timeEndPeriod(1);
            canvas_info.os_timed = false;
        }

        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int _canvas_window(int64_t x, int64_t y, int64_t width, int64_t height, const char *title)
{
    canvas_startup();

    int window_id = _canvas_get_free();
    if (window_id < 0)
        return window_id;

    DWORD style = WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_VISIBLE;

    canvas_info.canvas[window_id].index = window_id;

    HWND window = CreateWindowA("CanvasWindowClass", title, style, x, y, width, height, NULL, NULL, canvas_win32.instance, NULL);

    if (!window)
    {
        CANVAS_ERR("create win32 window");
        return CANVAS_ERR_GET_WINDOW;
    }

    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
    rid.usUsage = 0x02;     // HID_USAGE_GENERIC_MOUSE
    rid.dwFlags = 0;        // 0 = get input only when focused
    rid.hwndTarget = window;

    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
        CANVAS_WARN("failed to register raw input (error: %lu)\n", GetLastError());

    canvas_info.canvas[window_id].window = window;
    canvas_info.canvas[window_id].resize = false;
    canvas_info.canvas[window_id].titlebar = false;
    canvas_info.canvas[window_id].minimized = false;
    canvas_info.canvas[window_id].maximized = false;
    canvas_info.canvas[window_id].fullscreen = false;
    canvas_info.canvas[window_id]._valid = true;

    return window_id;
}

int _canvas_platform()
{
    CANVAS_ENTER_FUNC();

    canvas_win32.instance = GetModuleHandle(NULL);

    WNDCLASSA wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = canvas_win32.instance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "CanvasWindowClass";

    canvas_win32.class = RegisterClassA(&wc);

    if (!canvas_win32.class)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_PLATFORM, "register windows class failed");
    }

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_update()
{
    CANVAS_ENTER_FUNC();

    canvas_pointer *p = canvas_get_primary_pointer(0);

    POINT abs_screen_pt;
    if (GetCursorPos(&abs_screen_pt))
    {
        HMONITOR monitor = MonitorFromPoint(abs_screen_pt, MONITOR_DEFAULTTONEAREST);
        p->display = 0;

        for (int d = 0; d < canvas_info.display_count; d++)
        {
            if (canvas_win32.monitors[d] == monitor)
            {
                p->display = d;
                p->screen_x = abs_screen_pt.x - canvas_info.display[d].x;
                p->screen_y = abs_screen_pt.y - canvas_info.display[d].y;
                break;
            }
        }

        uint32_t old_buttons = p->buttons;
        uint32_t new_buttons = 0;

        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
            new_buttons |= CANVAS_BUTTON_LEFT;
        if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
            new_buttons |= CANVAS_BUTTON_RIGHT;
        if (GetAsyncKeyState(VK_MBUTTON) & 0x8000)
            new_buttons |= CANVAS_BUTTON_MIDDLE;
        if (GetAsyncKeyState(VK_XBUTTON1) & 0x8000)
            new_buttons |= CANVAS_BUTTON_X1;
        if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000)
            new_buttons |= CANVAS_BUTTON_X2;

        p->buttons_pressed = new_buttons & ~old_buttons;
        p->buttons_released = old_buttons & ~new_buttons;
        p->buttons = new_buttons;

        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        double timestamp = (double)counter.QuadPart / (double)_canvas_qpc_frequency.QuadPart;

        p->_samples[p->_sample_index].x = p->screen_x;
        p->_samples[p->_sample_index].y = p->screen_y;
        p->_samples[p->_sample_index].time = timestamp;
        p->_sample_index = (p->_sample_index + 1) % CANVAS_POINTER_SAMPLE_FRAMES;

        HWND hwnd_at_point = WindowFromPoint(abs_screen_pt);
        int active_window = -1;
        bool found_window = false;

        for (int i = 0; i < MAX_CANVAS; i++)
        {
            if (!canvas_info.canvas[i]._valid || !canvas_info.canvas[i].window)
                continue;

            HWND hwnd = (HWND)canvas_info.canvas[i].window;

            if (hwnd == hwnd_at_point || IsChild(hwnd, hwnd_at_point))
            {
                active_window = i;
                found_window = true;

                POINT client_pt = abs_screen_pt;
                ScreenToClient(hwnd, &client_pt);

                p->window_id = i;
                p->inside_window = true;
                p->x = client_pt.x;
                p->y = client_pt.y;

                break;
            }
        }

        if (!found_window)
        {
            p->inside_window = false;
            p->x = 0;
            p->y = 0;
        }
    }

    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (canvas_win32.device)
    {
        for (int i = 0; i < MAX_CANVAS; ++i)
        {
            if (canvas_info.canvas[i].resize)
            {
                _canvas_window_resize(i);
            }
        }

        canvas_win32.cmdAllocator->lpVtbl->Reset(canvas_win32.cmdAllocator);
        canvas_win32.cmdList->lpVtbl->Reset(canvas_win32.cmdList, canvas_win32.cmdAllocator, NULL);

        for (int i = 0; i < MAX_CANVAS; ++i)
        {
            if (canvas_info.canvas[i].window == NULL ||
                _canvas_data[i].swapChain == NULL ||
                _canvas_data[i].backBuffers[0] == NULL)
                continue;

            UINT backBufferIndex = _canvas_data[i].swapChain->lpVtbl->GetCurrentBackBufferIndex(_canvas_data[i].swapChain);

            D3D12_RESOURCE_BARRIER barrier = {0};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = _canvas_data[i].backBuffers[backBufferIndex];
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            canvas_win32.cmdList->lpVtbl->ResourceBarrier(canvas_win32.cmdList, 1, &barrier);

            canvas_win32.cmdList->lpVtbl->ClearRenderTargetView(canvas_win32.cmdList, _canvas_data[i].rtvHandles[backBufferIndex], canvas_info.canvas[i].clear, 0, NULL);

            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            canvas_win32.cmdList->lpVtbl->ResourceBarrier(canvas_win32.cmdList, 1, &barrier);
        }

        canvas_win32.cmdList->lpVtbl->Close(canvas_win32.cmdList);
        ID3D12CommandList *cmdLists[] = {(ID3D12CommandList *)canvas_win32.cmdList};
        canvas_win32.cmdQueue->lpVtbl->ExecuteCommandLists(canvas_win32.cmdQueue, 1, cmdLists);

        for (int i = 0; i < MAX_CANVAS; ++i)
        {
            if (_canvas_data[i].swapChain != NULL)
            {
                if (canvas_info.canvas[i].vsync)
                {
                    _canvas_data[i].swapChain->lpVtbl->Present(_canvas_data[i].swapChain, 1, 0);
                }
                else
                {
                    _canvas_data[i].swapChain->lpVtbl->Present(_canvas_data[i].swapChain, 0, DXGI_PRESENT_ALLOW_TEARING);
                }
            }
        }

        canvas_win32.fence_value++;
        canvas_win32.cmdQueue->lpVtbl->Signal(canvas_win32.cmdQueue, canvas_win32.fence, canvas_win32.fence_value);
        if (canvas_win32.fence->lpVtbl->GetCompletedValue(canvas_win32.fence) < canvas_win32.fence_value)
        {
            canvas_win32.fence->lpVtbl->SetEventOnCompletion(canvas_win32.fence, canvas_win32.fence_value, canvas_win32.fence_event);
            WaitForSingleObject(canvas_win32.fence_event, INFINITE);
        }
    }

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_gpu_init()
{
    CANVAS_ENTER_FUNC();

    if (canvas_info.init_gpu)
    {
        CANVAS_RETURN(CANVAS_OK);
    }

    canvas_info.init_gpu = 1;

    HRESULT result;

    result = CreateDXGIFactory1(&IID_IDXGIFactory4, (void **)&canvas_win32.factory);
    if (FAILED(result))
    {
        CANVAS_ERR("create dx12 factory failed (HRESULT: 0x%08lX)\n", result);
        return CANVAS_ERR_GET_GPU;
    }

    result = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, (void **)&canvas_win32.device);
    if (FAILED(result))
    {
        CANVAS_ERR("create dx12 device failed (HRESULT: 0x%08lX)\n", result);
        canvas_win32.factory->lpVtbl->Release(canvas_win32.factory);
        canvas_win32.factory = NULL;
        return CANVAS_ERR_GET_GPU;
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {0};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    result = canvas_win32.device->lpVtbl->CreateCommandQueue(canvas_win32.device, &queueDesc, &IID_ID3D12CommandQueue, (void **)&canvas_win32.cmdQueue);
    if (FAILED(result))
    {
        CANVAS_ERR("create command queue failed (HRESULT: 0x%08lX)\n", result);
        goto cleanup_gpu;
    }

    result = canvas_win32.device->lpVtbl->CreateCommandAllocator(canvas_win32.device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, (void **)&canvas_win32.cmdAllocator);
    if (FAILED(result))
    {
        CANVAS_ERR("create command allocator failed (HRESULT: 0x%08lX)\n", result);
        goto cleanup_gpu;
    }

    result = canvas_win32.device->lpVtbl->CreateCommandList(canvas_win32.device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, canvas_win32.cmdAllocator, NULL, &IID_ID3D12GraphicsCommandList, (void **)&canvas_win32.cmdList);
    if (FAILED(result))
    {
        CANVAS_ERR("create command list failed (HRESULT: 0x%08lX)\n", result);
        goto cleanup_gpu;
    }

    canvas_win32.cmdList->lpVtbl->Close(canvas_win32.cmdList);

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {0};
    heapDesc.NumDescriptors = MAX_CANVAS * 2;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    result = canvas_win32.device->lpVtbl->CreateDescriptorHeap(canvas_win32.device, &heapDesc, &IID_ID3D12DescriptorHeap, (void **)&canvas_win32.rtvHeap);
    if (FAILED(result))
    {
        CANVAS_ERR("create descriptor heap failed (HRESULT: 0x%08lX)\n", result);
        goto cleanup_gpu;
    }

    canvas_win32.rtvDescriptorSize = canvas_win32.device->lpVtbl->GetDescriptorHandleIncrementSize(canvas_win32.device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    result = canvas_win32.device->lpVtbl->CreateFence(canvas_win32.device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, (void **)&canvas_win32.fence);
    if (FAILED(result))
    {
        CANVAS_ERR("create fence failed (HRESULT: 0x%08lX)\n", result);
        goto cleanup_gpu;
    }

    canvas_win32.fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!canvas_win32.fence_event)
    {
        CANVAS_ERR("create fence event failed (GetLastError: %lu)\n", GetLastError());
        goto cleanup_gpu;
    }

    return CANVAS_OK;

cleanup_gpu:
    if (canvas_win32.fence)
    {
        canvas_win32.fence->lpVtbl->Release(canvas_win32.fence);
        canvas_win32.fence = NULL;
    }
    if (canvas_win32.rtvHeap)
    {
        canvas_win32.rtvHeap->lpVtbl->Release(canvas_win32.rtvHeap);
        canvas_win32.rtvHeap = NULL;
    }
    if (canvas_win32.cmdList)
    {
        canvas_win32.cmdList->lpVtbl->Release(canvas_win32.cmdList);
        canvas_win32.cmdList = NULL;
    }
    if (canvas_win32.cmdAllocator)
    {
        canvas_win32.cmdAllocator->lpVtbl->Release(canvas_win32.cmdAllocator);
        canvas_win32.cmdAllocator = NULL;
    }
    if (canvas_win32.cmdQueue)
    {
        canvas_win32.cmdQueue->lpVtbl->Release(canvas_win32.cmdQueue);
        canvas_win32.cmdQueue = NULL;
    }
    if (canvas_win32.device)
    {
        canvas_win32.device->lpVtbl->Release(canvas_win32.device);
        canvas_win32.device = NULL;
    }
    if (canvas_win32.factory)
    {
        canvas_win32.factory->lpVtbl->Release(canvas_win32.factory);
        canvas_win32.factory = NULL;
    }

    canvas_info.init_gpu = 0;
    CANVAS_RETURN(CANVAS_ERR_GET_GPU);
}

int _canvas_gpu_new_window(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    _canvas_data[window_id] = (canvas_data){0};

    HWND window = (HWND)canvas_info.canvas[window_id].window;

    RECT rect;
    GetClientRect(window, &rect);

    DXGI_SWAP_CHAIN_DESC1 scDesc = {0};
    scDesc.Width = rect.right - rect.left;
    scDesc.Height = rect.bottom - rect.top;
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.SampleDesc.Count = 1;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.BufferCount = 2;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    scDesc.Scaling = DXGI_SCALING_STRETCH;
    scDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

    IDXGISwapChain1 *swapChain1;
    HRESULT result = canvas_win32.factory->lpVtbl->CreateSwapChainForHwnd(
        canvas_win32.factory,
        (IUnknown *)canvas_win32.cmdQueue,
        window,
        &scDesc,
        NULL,
        NULL,
        &swapChain1);

    if (FAILED(result))
    {
        CANVAS_ERR("create swapchain for hwnd\n");
        return CANVAS_ERR_GET_GPU;
    }

    result = swapChain1->lpVtbl->QueryInterface(
        swapChain1,
        &IID_IDXGISwapChain3,
        (void **)&_canvas_data[window_id].swapChain);
    swapChain1->lpVtbl->Release(swapChain1);

    if (FAILED(result))
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_GPU, "query swapchain interface\n");
    }

    canvas_win32.factory->lpVtbl->MakeWindowAssociation(
        canvas_win32.factory,
        window,
        DXGI_MWA_NO_ALT_ENTER);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    canvas_win32.rtvHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(canvas_win32.rtvHeap, &rtvHandle);

    rtvHandle.ptr += window_id * 2 * canvas_win32.rtvDescriptorSize;

    for (int i = 0; i < 2; i++)
    {
        result = _canvas_data[window_id].swapChain->lpVtbl->GetBuffer(
            _canvas_data[window_id].swapChain,
            i,
            &IID_ID3D12Resource,
            (void **)&_canvas_data[window_id].backBuffers[i]);

        if (FAILED(result))
        {
            CANVAS_RETURN_ERR(CANVAS_ERR_GET_GPU, "get swapchain buffer %d\n", i);
        }

        _canvas_data[window_id].rtvHandles[i] = rtvHandle;
        canvas_win32.device->lpVtbl->CreateRenderTargetView(
            canvas_win32.device,
            _canvas_data[window_id].backBuffers[i],
            NULL,
            rtvHandle);
        rtvHandle.ptr += canvas_win32.rtvDescriptorSize;
    }

    // Create depth stencil texture
    D3D12_HEAP_PROPERTIES depthHeapProps = {0};
    depthHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC depthDesc = {0};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = rect.right - rect.left;
    depthDesc.Height = rect.bottom - rect.top;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depthClearValue = {0};
    depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthClearValue.DepthStencil.Depth = 1.0f;

    ID3D12Resource *depthTexture;
    result = canvas_win32.device->lpVtbl->CreateCommittedResource(
        canvas_win32.device,
        &depthHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue,
        &IID_ID3D12Resource,
        (void **)&depthTexture);

    if (FAILED(result))
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_GPU, "failed to create depth texture (HRESULT: 0x%08lX)\n", result);
    }

    canvas_info.canvas[window_id].depth_texture = depthTexture;

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_window_resize(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    canvas_data *window = &_canvas_data[window_id];

    if (!window->swapChain || !canvas_info.canvas[window_id].resize)
    {
        canvas_info.canvas[window_id].resize = false;
        CANVAS_RETURN(CANVAS_OK);
    }

    canvas_info.canvas[window_id].resize = false;

    canvas_win32.fence_value++;
    canvas_win32.cmdQueue->lpVtbl->Signal(canvas_win32.cmdQueue, canvas_win32.fence, canvas_win32.fence_value);
    if (canvas_win32.fence->lpVtbl->GetCompletedValue(canvas_win32.fence) < canvas_win32.fence_value)
    {
        canvas_win32.fence->lpVtbl->SetEventOnCompletion(canvas_win32.fence, canvas_win32.fence_value, canvas_win32.fence_event);
        WaitForSingleObject(canvas_win32.fence_event, INFINITE);
    }

    RECT rect;
    GetClientRect((HWND)canvas_info.canvas[window_id].window, &rect);
    UINT width = rect.right - rect.left;
    UINT height = rect.bottom - rect.top;

    if (width == 0 || height == 0)
    {
        CANVAS_RETURN(CANVAS_OK);
    }

    for (int i = 0; i < 2; i++)
    {
        if (window->backBuffers[i])
        {
            window->backBuffers[i]->lpVtbl->Release(window->backBuffers[i]);
            window->backBuffers[i] = NULL;
        }
    }

    HRESULT hr = window->swapChain->lpVtbl->ResizeBuffers(
        window->swapChain, 2, width, height,
        DXGI_FORMAT_UNKNOWN,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);

    if (FAILED(hr))
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_GPU, "resize swapchain buffers\n");
    }

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    canvas_win32.rtvHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(canvas_win32.rtvHeap, &rtvHandle);
    rtvHandle.ptr += window_id * 2 * canvas_win32.rtvDescriptorSize;

    for (int i = 0; i < 2; i++)
    {
        hr = window->swapChain->lpVtbl->GetBuffer(
            window->swapChain, i, &IID_ID3D12Resource,
            (void **)&window->backBuffers[i]);

        if (FAILED(hr))
        {
            CANVAS_RETURN_ERR(CANVAS_ERR_GET_GPU, "get resized buffer %d\n", i);
        }

        window->rtvHandles[i] = rtvHandle;
        canvas_win32.device->lpVtbl->CreateRenderTargetView(
            canvas_win32.device, window->backBuffers[i], NULL, rtvHandle);
        rtvHandle.ptr += canvas_win32.rtvDescriptorSize;
    }

    if (canvas_info.canvas[window_id].depth_texture)
    {
        ID3D12Resource *old_depth = (ID3D12Resource *)canvas_info.canvas[window_id].depth_texture;
        old_depth->lpVtbl->Release(old_depth);
        canvas_info.canvas[window_id].depth_texture = NULL;
    }

    D3D12_HEAP_PROPERTIES depthHeapProps = {0};
    depthHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC depthDesc = {0};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depthClearValue = {0};
    depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthClearValue.DepthStencil.Depth = 1.0f;

    ID3D12Resource *depthTexture;
    hr = canvas_win32.device->lpVtbl->CreateCommittedResource(
        canvas_win32.device,
        &depthHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue,
        &IID_ID3D12Resource,
        (void **)&depthTexture);

    if (SUCCEEDED(hr))
    {
        canvas_info.canvas[window_id].depth_texture = depthTexture;
    }
    else
    {
        CANVAS_WARN("failed to recreate depth texture (HRESULT: 0x%08lX)\n", hr);
    }

    CANVAS_RETURN(CANVAS_OK);
}
static int _d3d12_upload_buffer_data(canvas_buffer *buf, void *data, size_t size)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT(buf != NULL);
    CANVAS_ASSERT(data != NULL);
    CANVAS_ASSERT(size > 0);

    D3D12_HEAP_PROPERTIES upload_heap = {0};
    upload_heap.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC staging_desc = {0};
    staging_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    staging_desc.Width = size;
    staging_desc.Height = 1;
    staging_desc.DepthOrArraySize = 1;
    staging_desc.MipLevels = 1;
    staging_desc.Format = DXGI_FORMAT_UNKNOWN;
    staging_desc.SampleDesc.Count = 1;
    staging_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ID3D12Resource *staging = NULL;
    HRESULT hr = canvas_win32.device->lpVtbl->CreateCommittedResource(
        canvas_win32.device,
        &upload_heap,
        D3D12_HEAP_FLAG_NONE,
        &staging_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        NULL,
        &IID_ID3D12Resource,
        (void **)&staging);

    if (FAILED(hr))
    {
        CANVAS_RETURN_ERR(CANVAS_FAIL, "Failed to create staging buffer\n");
    }

    // Map and copy data to staging
    void *mapped = NULL;
    D3D12_RANGE read_range = {0, 0};
    hr = staging->lpVtbl->Map(staging, 0, &read_range, &mapped);
    if (FAILED(hr))
    {
        staging->lpVtbl->Release(staging);
        CANVAS_RETURN(CANVAS_FAIL);
    }

    memcpy(mapped, data, size);
    staging->lpVtbl->Unmap(staging, 0, NULL);

    // Wait for previous frames to finish before resetting command allocator
    canvas_win32.fence_value++;
    canvas_win32.cmdQueue->lpVtbl->Signal(canvas_win32.cmdQueue, canvas_win32.fence, canvas_win32.fence_value);
    if (canvas_win32.fence->lpVtbl->GetCompletedValue(canvas_win32.fence) < canvas_win32.fence_value)
    {
        canvas_win32.fence->lpVtbl->SetEventOnCompletion(canvas_win32.fence, canvas_win32.fence_value, canvas_win32.fence_event);
        WaitForSingleObject(canvas_win32.fence_event, INFINITE);
    }

    // Reset command allocator and list
    canvas_win32.cmdAllocator->lpVtbl->Reset(canvas_win32.cmdAllocator);
    canvas_win32.cmdList->lpVtbl->Reset(canvas_win32.cmdList,
                                        canvas_win32.cmdAllocator, NULL);

    // Transition destination buffer to COPY_DEST
    ID3D12Resource *dst = (ID3D12Resource *)buf->platform_handle;

    D3D12_RESOURCE_BARRIER barrier = {0};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = dst;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    canvas_win32.cmdList->lpVtbl->ResourceBarrier(canvas_win32.cmdList, 1, &barrier);

    // Copy staging -> destination
    canvas_win32.cmdList->lpVtbl->CopyResource(canvas_win32.cmdList, dst, staging);

    // Transition to appropriate state based on buffer type
    D3D12_RESOURCE_STATES final_state;
    switch (buf->type)
    {
    case CANVAS_BUFFER_VERTEX:
        final_state = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        break;
    case CANVAS_BUFFER_INDEX:
        final_state = D3D12_RESOURCE_STATE_INDEX_BUFFER;
        break;
    case CANVAS_BUFFER_UNIFORM:
        final_state = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        break;
    case CANVAS_BUFFER_STORAGE:
        final_state = D3D12_RESOURCE_STATE_COMMON;
        break;
    default:
        final_state = D3D12_RESOURCE_STATE_COMMON;
    }

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = final_state;
    canvas_win32.cmdList->lpVtbl->ResourceBarrier(canvas_win32.cmdList, 1, &barrier);

    // Close and execute
    canvas_win32.cmdList->lpVtbl->Close(canvas_win32.cmdList);

    ID3D12CommandList *cmdLists[] = {(ID3D12CommandList *)canvas_win32.cmdList};
    canvas_win32.cmdQueue->lpVtbl->ExecuteCommandLists(canvas_win32.cmdQueue, 1, cmdLists);

    // Wait for completion
    canvas_win32.fence_value++;
    canvas_win32.cmdQueue->lpVtbl->Signal(canvas_win32.cmdQueue, canvas_win32.fence, canvas_win32.fence_value);

    if (canvas_win32.fence->lpVtbl->GetCompletedValue(canvas_win32.fence) < canvas_win32.fence_value)
    {
        canvas_win32.fence->lpVtbl->SetEventOnCompletion(canvas_win32.fence, canvas_win32.fence_value, canvas_win32.fence_event);
        WaitForSingleObject(canvas_win32.fence_event, INFINITE);
    }

    staging->lpVtbl->Release(staging);

    CANVAS_RETURN(CANVAS_OK);
}

canvas_buffer *canvas_buffer_create(int window_id, canvas_buffer_type type, canvas_buffer_usage usage, size_t size, void *initial_data)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);
    CANVAS_ASSERT(size > 0);

    CANVAS_VALID_PTR(window_id);

    if (!canvas_win32.device)
    {
        CANVAS_RETURN_ERR(NULL, "GPU not initialized\n");
    }

    canvas_buffer *buf = (canvas_buffer *)calloc(1, sizeof(canvas_buffer));
    if (!buf)
    {
        CANVAS_RETURN(NULL);
    }

    buf->size = size;
    buf->type = type;
    buf->usage = usage;
    buf->window_id = window_id;

    // Determine heap type
    D3D12_HEAP_TYPE heap_type;
    D3D12_RESOURCE_STATES initial_state;

    if (usage == CANVAS_BUFFER_DYNAMIC)
    {
        heap_type = D3D12_HEAP_TYPE_UPLOAD;
        initial_state = D3D12_RESOURCE_STATE_GENERIC_READ;
    }
    else
    {
        heap_type = D3D12_HEAP_TYPE_DEFAULT;
        initial_state = D3D12_RESOURCE_STATE_COMMON;
    }

    // Create buffer
    D3D12_HEAP_PROPERTIES heap_props = {0};
    heap_props.Type = heap_type;
    heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resource_desc = {0};
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resource_desc.Width = size;
    resource_desc.Height = 1;
    resource_desc.DepthOrArraySize = 1;
    resource_desc.MipLevels = 1;
    resource_desc.Format = DXGI_FORMAT_UNKNOWN;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource *resource = NULL;
    HRESULT hr = canvas_win32.device->lpVtbl->CreateCommittedResource(
        canvas_win32.device,
        &heap_props,
        D3D12_HEAP_FLAG_NONE,
        &resource_desc,
        initial_state,
        NULL,
        &IID_ID3D12Resource,
        (void **)&resource);

    if (FAILED(hr))
    {
        CANVAS_ERR("Failed to create D3D12 buffer (HRESULT: 0x%08lX)\n", hr);
        free(buf);
        CANVAS_RETURN(NULL);
    }

    buf->platform_handle = resource;

    // Persistent mapping for dynamic buffers
    if (usage == CANVAS_BUFFER_DYNAMIC)
    {
        D3D12_RANGE read_range = {0, 0};
        hr = resource->lpVtbl->Map(resource, 0, &read_range, &buf->mapped);

        if (FAILED(hr))
        {
            CANVAS_ERR("Failed to map D3D12 buffer\n");
            resource->lpVtbl->Release(resource);
            free(buf);
            CANVAS_RETURN(NULL);
        }

        if (initial_data)
        {
            memcpy(buf->mapped, initial_data, size);
        }
    }
    else if (initial_data)
    {
        if (_d3d12_upload_buffer_data(buf, initial_data, size) != CANVAS_OK)
        {
            CANVAS_ERR("Failed to upload static buffer data\n");
            resource->lpVtbl->Release(resource);
            free(buf);
            CANVAS_RETURN(NULL);
        }
    }

    CANVAS_RETURN(buf);
}

void canvas_buffer_update(canvas_buffer *buf, void *data, size_t size, size_t offset)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(buf);
    CANVAS_ASSERT_NOT_NULL(data);
    CANVAS_ASSERT(size > 0);

    if (!buf || !data)
    {
        CANVAS_RETURN_VOID();
    }

    if (buf->usage != CANVAS_BUFFER_DYNAMIC)
    {
        CANVAS_WARN("Can only update dynamic buffers\n");
        CANVAS_RETURN_VOID();
    }

    if (offset > buf->size || size > buf->size - offset)
    {
        CANVAS_ERR("Buffer update out of bounds\n");
        CANVAS_RETURN_VOID();
    }

    memcpy((uint8_t *)buf->mapped + offset, data, size);
    CANVAS_RETURN_VOID();
}

void *canvas_buffer_map(canvas_buffer *buf)
{
    CANVAS_ENTER_FUNC();

    if (!buf)
    {
        CANVAS_RETURN(NULL);
    }

    if (buf->usage == CANVAS_BUFFER_DYNAMIC)
    {
        CANVAS_RETURN(buf->mapped);
    }

    ID3D12Resource *resource = (ID3D12Resource *)buf->platform_handle;
    D3D12_RANGE read_range = {0, 0};

    void *mapped = NULL;
    HRESULT hr = resource->lpVtbl->Map(resource, 0, &read_range, &mapped);

    if (FAILED(hr))
    {
        CANVAS_RETURN_ERR(NULL, "Failed to map buffer\n");
    }

    CANVAS_RETURN(mapped);
}

void canvas_buffer_unmap(canvas_buffer *buf)
{
    CANVAS_ENTER_FUNC();

    if (!buf)
    {
        CANVAS_RETURN_VOID();
    }

    if (buf->usage == CANVAS_BUFFER_DYNAMIC)
    {
        CANVAS_RETURN_VOID();
    }

    ID3D12Resource *resource = (ID3D12Resource *)buf->platform_handle;
    resource->lpVtbl->Unmap(resource, 0, NULL);
    CANVAS_RETURN_VOID();
}

void canvas_buffer_destroy(canvas_buffer *buf)
{
    CANVAS_ENTER_FUNC();

    if (!buf)
    {
        CANVAS_RETURN_VOID();
    }

    ID3D12Resource *resource = (ID3D12Resource *)buf->platform_handle;

    if (buf->mapped && buf->usage == CANVAS_BUFFER_DYNAMIC)
        resource->lpVtbl->Unmap(resource, 0, NULL);

    resource->lpVtbl->Release(resource);
    free(buf);
    CANVAS_RETURN_VOID();
}

int _canvas_post_update()
{
    CANVAS_ENTER_FUNC();
    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_exit()
{
    CANVAS_ENTER_FUNC();
    if (canvas_win32.fence_event)
        CloseHandle(canvas_win32.fence_event);
    if (canvas_win32.fence)
        canvas_win32.fence->lpVtbl->Release(canvas_win32.fence);
    if (canvas_win32.rtvHeap)
        canvas_win32.rtvHeap->lpVtbl->Release(canvas_win32.rtvHeap);
    if (canvas_win32.cmdList)
        canvas_win32.cmdList->lpVtbl->Release(canvas_win32.cmdList);
    if (canvas_win32.cmdAllocator)
        canvas_win32.cmdAllocator->lpVtbl->Release(canvas_win32.cmdAllocator);
    if (canvas_win32.cmdQueue)
        canvas_win32.cmdQueue->lpVtbl->Release(canvas_win32.cmdQueue);
    if (canvas_win32.device)
        canvas_win32.device->lpVtbl->Release(canvas_win32.device);
    if (canvas_win32.factory)
        canvas_win32.factory->lpVtbl->Release(canvas_win32.factory);
    if (canvas_win32.class)
        UnregisterClassA("CanvasWindowClass", canvas_win32.instance);
    CANVAS_RETURN(CANVAS_OK);
}

#endif

//
//
//
#if defined(__linux__)

int canvas_minimize(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!x11.display)
        {
            CANVAS_RETURN_ERR(CANVAS_ERR_GET_DISPLAY, "no display connection\n");
        }

        Window window = (Window)canvas_info.canvas[window_id].window;
        if (!window)
        {
            CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to minimize: %d\n", window_id);
        }

        int screen = x11.XDefaultScreen(x11.display);
        x11.XIconifyWindow(x11.display, window, screen);
        x11.XFlush(x11.display);
    }

    canvas_info.canvas[window_id].minimized = true;
    canvas_info.canvas[window_id].maximized = false;

    CANVAS_RETURN(CANVAS_OK);
}

int canvas_maximize(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!x11.display)
        {
            CANVAS_RETURN_ERR(CANVAS_ERR_GET_DISPLAY, "no display connection\n");
        }

        Window window = (Window)canvas_info.canvas[window_id].window;
        if (!window)
        {
            CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to maximize: %d\n", window_id);
        }

        int screen = x11.XDefaultScreen(x11.display);

        Atom wm_state = x11.XInternAtom(x11.display, "_NET_WM_STATE", 0);
        Atom max_horz = x11.XInternAtom(x11.display, "_NET_WM_STATE_MAXIMIZED_HORZ", 0);
        Atom max_vert = x11.XInternAtom(x11.display, "_NET_WM_STATE_MAXIMIZED_VERT", 0);

        XClientMessageEvent ev = {0};
        ev.type = 33;
        ev.window = window;
        ev.message_type = wm_state;
        ev.format = 32;
        ev.data.l[0] = 1;
        ev.data.l[1] = max_horz;
        ev.data.l[2] = max_vert;
        ev.data.l[3] = 1;

        x11.XSendEvent(x11.display, x11.XRootWindow(x11.display, screen),
                       0, (1L << 20) | (1L << 19), (XEvent *)&ev);
        x11.XFlush(x11.display);
    }

    canvas_info.canvas[window_id].maximized = true;
    canvas_info.canvas[window_id].minimized = false;

    CANVAS_RETURN(CANVAS_OK);
}

int canvas_restore(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!x11.display)
        {
            CANVAS_RETURN_ERR(CANVAS_ERR_GET_DISPLAY, "no display connection\n");
        }

        Window window = (Window)canvas_info.canvas[window_id].window;
        if (!window)
        {
            CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to restore: %d\n", window_id);
        }

        int screen = x11.XDefaultScreen(x11.display);

        if (canvas_info.canvas[window_id].minimized)
        {
            x11.XMapWindow(x11.display, window);
        }
        else if (canvas_info.canvas[window_id].maximized)
        {
            Atom wm_state = x11.XInternAtom(x11.display, "_NET_WM_STATE", 0);
            Atom max_horz = x11.XInternAtom(x11.display, "_NET_WM_STATE_MAXIMIZED_HORZ", 0);
            Atom max_vert = x11.XInternAtom(x11.display, "_NET_WM_STATE_MAXIMIZED_VERT", 0);

            XClientMessageEvent ev = {0};
            ev.type = 33;
            ev.window = window;
            ev.message_type = wm_state;
            ev.format = 32;
            ev.data.l[0] = 0;
            ev.data.l[1] = max_horz;
            ev.data.l[2] = max_vert;
            ev.data.l[3] = 1;

            x11.XSendEvent(x11.display, x11.XRootWindow(x11.display, screen),
                           0, (1L << 20) | (1L << 19), (XEvent *)&ev);
        }
        else if (canvas_info.canvas[window_id].fullscreen)
        {
            Atom wm_state = x11.XInternAtom(x11.display, "_NET_WM_STATE", 0);
            Atom fullscreen = x11.XInternAtom(x11.display, "_NET_WM_STATE_FULLSCREEN", 0);

            XClientMessageEvent ev = {0};
            ev.type = 33;
            ev.window = window;
            ev.message_type = wm_state;
            ev.format = 32;
            ev.data.l[0] = 0; // _NET_WM_STATE_REMOVE
            ev.data.l[1] = fullscreen;
            ev.data.l[2] = 0;
            ev.data.l[3] = 1;

            x11.XSendEvent(x11.display, x11.XRootWindow(x11.display, screen), 0, (1L << 20) | (1L << 19), (XEvent *)&ev);

            x11.XMoveResizeWindow(x11.display, window,
                                  (int)_canvas_data[window_id].saved_x,
                                  (int)_canvas_data[window_id].saved_y,
                                  (unsigned int)_canvas_data[window_id].saved_width,
                                  (unsigned int)_canvas_data[window_id].saved_height);
        }

        x11.XFlush(x11.display);
    }

    canvas_info.canvas[window_id].minimized = false;
    canvas_info.canvas[window_id].maximized = false;
    canvas_info.canvas[window_id].fullscreen = false;

    CANVAS_RETURN(CANVAS_OK);
}

int canvas_fullscreen(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    if (_canvas_using_wayland)
    {
        // TODO: Wayland fullscreen support
        CANVAS_WARN("Wayland fullscreen not yet implemented\n");
        CANVAS_RETURN(CANVAS_OK);
    }
    else
    {
        if (!x11.display)
        {
            CANVAS_RETURN_ERR(CANVAS_ERR_GET_DISPLAY, "no display connection\n");
        }

        Window window = (Window)canvas_info.canvas[window_id].window;
        if (!window)
        {
            CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window for fullscreen: %d\n", window_id);
        }

        if (!canvas_info.canvas[window_id].fullscreen)
        {
            XWindowAttributes attrs;
            x11.XGetWindowAttributes(x11.display, window, &attrs);
            _canvas_data[window_id].saved_x = attrs.x;
            _canvas_data[window_id].saved_y = attrs.y;
            _canvas_data[window_id].saved_width = attrs.width;
            _canvas_data[window_id].saved_height = attrs.height;

            int screen = x11.XDefaultScreen(x11.display);
            Atom wm_state = x11.XInternAtom(x11.display, "_NET_WM_STATE", 0);
            Atom fullscreen = x11.XInternAtom(x11.display, "_NET_WM_STATE_FULLSCREEN", 0);

            XClientMessageEvent ev = {0};
            ev.type = 33;
            ev.window = window;
            ev.message_type = wm_state;
            ev.format = 32;
            ev.data.l[0] = 1; // _NET_WM_STATE_ADD
            ev.data.l[1] = fullscreen;
            ev.data.l[2] = 0;
            ev.data.l[3] = 1;

            x11.XSendEvent(x11.display, x11.XRootWindow(x11.display, screen), 0, (1L << 20) | (1L << 19), (XEvent *)&ev);
            x11.XFlush(x11.display);

            canvas_info.canvas[window_id].fullscreen = true;
        }
    }

    canvas_info.canvas[window_id].minimized = false;
    canvas_info.canvas[window_id].maximized = false;

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_close(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    if (vk_windows[window_id].initialized)
    {
        vk_cleanup_window(window_id);
    }

    if (_canvas_using_wayland)
    {
    }
    else
    {
        Window window = (Window)canvas_info.canvas[window_id].window;

        if (!window)
        {
            CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "no window to close: %d\n", window_id);
        }

        x11.XDestroyWindow(x11.display, window);
        x11.XFlush(x11.display);
    }

    _canvas_data[window_id].last_button_press_time = 0;
    _canvas_data[window_id].last_button_press_x = 0;
    _canvas_data[window_id].last_button_press_y = 0;

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_set(int window_id, int display, int64_t x, int64_t y, int64_t width, int64_t height, const char *title)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);
    CANVAS_DISPLAY_BOUNDS(display);

    if (width <= 0 || height <= 0)
    {
        CANVAS_RETURN_ERR(CANVAS_FAIL, "invalid window dimensions: %dx%d\n", width, height);
    }

    if (_canvas_using_wayland)
    {
    }
    else
    {
        Window window = (Window)canvas_info.canvas[window_id].window;

        if (title && strcmp(canvas_info.canvas[window_id].title, title) != 0)
        {
            x11.XStoreName(x11.display, window, title);
            Atom net_wm_name = x11.XInternAtom(x11.display, "_NET_WM_NAME", 0);
            Atom utf8_string = x11.XInternAtom(x11.display, "UTF8_STRING", 0);
            x11.XChangeProperty(x11.display, window, net_wm_name, utf8_string, 8, 0,
                                (unsigned char *)title, strlen(title));
        }

        _canvas_data[window_id].client_set = true;

        int global_x = canvas_info.display[display].x + x;
        int global_y = canvas_info.display[display].y + y;

        if (global_x < -32768 || global_x > 32767 || global_y < -32768 || global_y > 32767)
        {
            CANVAS_RETURN_ERR(CANVAS_FAIL, "window position out of range: %d,%d\n", global_x, global_y);
        }

        x11.XMoveResizeWindow(x11.display, window,
                              (int)global_x, (int)global_y,
                              (unsigned int)width, (unsigned int)height);

        x11.XFlush(x11.display);
    }

    canvas_info.canvas[window_id].os_moved = false;
    canvas_info.canvas[window_id].os_resized = false;

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_init_displays()
{
    CANVAS_ENTER_FUNC();
    canvas_info.display_count = 0;

    if (_canvas_using_wayland)
    {
        CANVAS_RETURN(canvas_info.display_count);
    }

    if (xrandr.library)
    {
        Window root = x11.XDefaultRootWindow(x11.display);
        XRRScreenResources *sr = xrandr.XRRGetScreenResourcesCurrent(x11.display, root);

        if (!sr)
        {
            CANVAS_WARN("XRRGetScreenResourcesCurrent failed\n");
            dlclose(xrandr.library);
            xrandr.library = NULL;
            goto fallback;
        }

        unsigned long primary_output = xrandr.XRRGetOutputPrimary(x11.display, root);

        for (int i = 0; i < sr->ncrtc; i++)
        {
            if (canvas_info.display_count >= MAX_DISPLAYS)
                break;

            XRRCrtcInfo *ci = xrandr.XRRGetCrtcInfo(x11.display, sr, sr->crtcs[i]);

            if (!ci || ci->width == 0 || ci->height == 0 || ci->noutput == 0)
            {
                if (ci)
                    xrandr.XRRFreeCrtcInfo(ci);
                continue;
            }

            XRROutputInfo *oi = xrandr.XRRGetOutputInfo(x11.display, sr, ci->outputs[0]);

            if (!oi)
            {
                xrandr.XRRFreeCrtcInfo(ci);
                continue;
            }

            double scale = 1.0;
            if (oi->mm_width > 0 && oi->mm_height > 0)
            {
                double dpi_x = (double)ci->width / (oi->mm_width / 25.4);
                double dpi_y = (double)ci->height / (oi->mm_height / 25.4);
                double dpi = (dpi_x + dpi_y) / 2.0;

                if (dpi > 140.0)
                {
                    scale = dpi / 96.0;
                }
            }

            int refresh_rate = 60;
            if (ci->mode != 0)
            {
                XRRModeInfo *modes = (XRRModeInfo *)sr->modes;
                for (int j = 0; j < sr->nmode; j++)
                {
                    if (modes[j].id == ci->mode)
                    {
                        if (modes[j].hTotal > 0 && modes[j].vTotal > 0)
                        {
                            double rate = (double)modes[j].dotClock /
                                          ((double)modes[j].hTotal * (double)modes[j].vTotal);
                            refresh_rate = (int)(rate + 0.5);
                        }
                        break;
                    }
                }
            }

            canvas_info.display[canvas_info.display_count].x = ci->x;
            canvas_info.display[canvas_info.display_count].y = ci->y;
            canvas_info.display[canvas_info.display_count].width = ci->width;
            canvas_info.display[canvas_info.display_count].height = ci->height;
            canvas_info.display[canvas_info.display_count].refresh_rate = refresh_rate;
            canvas_info.display[canvas_info.display_count].scale = scale;
            canvas_info.display[canvas_info.display_count].primary = (ci->outputs[0] == primary_output);

            if (refresh_rate > canvas_info.highest_refresh_rate)
                canvas_info.highest_refresh_rate = refresh_rate;

            canvas_info.display_count++;

            xrandr.XRRFreeOutputInfo(oi);
            xrandr.XRRFreeCrtcInfo(ci);
        }

        xrandr.XRRFreeScreenResources(sr);

        if (canvas_info.display_count > 0)
        {
            dlclose(xrandr.library);
            xrandr.library = NULL;
            CANVAS_RETURN(canvas_info.display_count);
        }

        dlclose(xrandr.library);
        xrandr.library = NULL;
    }

fallback:

    CANVAS_WARN("using basic X11 (single display, limited info)\n");
    int screen = x11.XDefaultScreen(x11.display);

    canvas_info.display[0].primary = true;
    canvas_info.display[0].x = 0;
    canvas_info.display[0].y = 0;
    canvas_info.display[0].width = x11.XDisplayWidth(x11.display, screen);
    canvas_info.display[0].height = x11.XDisplayHeight(x11.display, screen);
    canvas_info.display[0].refresh_rate = 60;
    canvas_info.display[0].scale = 1.0;
    canvas_info.display_count = 1;

    CANVAS_RETURN(canvas_info.display_count);
}

int _canvas_get_window_display(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    if (_canvas_using_wayland)
    {
    }
    else
    {
    }

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_get_resize_edge_action(int window_id, int x, int y)
{
    CANVAS_ENTER_FUNC();
    if (window_id < 0 || window_id >= MAX_CANVAS)
        CANVAS_RETURN(-1);

    int width = canvas_info.canvas[window_id].width;
    int height = canvas_info.canvas[window_id].height;
    int border = 8;

    bool at_left = x < border;
    bool at_right = x > width - border;
    bool at_top = y < border;
    bool at_bottom = y > height - border;

    if (at_top && at_left)
        CANVAS_RETURN(_NET_WM_MOVERESIZE_SIZE_TOPLEFT);
    if (at_top && at_right)
        CANVAS_RETURN(_NET_WM_MOVERESIZE_SIZE_TOPRIGHT);
    if (at_bottom && at_left)
        CANVAS_RETURN(_NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT);
    if (at_bottom && at_right)
        CANVAS_RETURN(_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT);

    if (at_top)
        CANVAS_RETURN(_NET_WM_MOVERESIZE_SIZE_TOP);
    if (at_bottom)
        CANVAS_RETURN(_NET_WM_MOVERESIZE_SIZE_BOTTOM);
    if (at_left)
        CANVAS_RETURN(_NET_WM_MOVERESIZE_SIZE_LEFT);
    if (at_right)
        CANVAS_RETURN(_NET_WM_MOVERESIZE_SIZE_RIGHT);

    CANVAS_RETURN(-1);
}

void _canvas_start_wm_move_resize(int window_id, int x_root, int y_root, int action)
{
    CANVAS_ENTER_FUNC();
    XClientMessageEvent ev = {0};
    ev.type = X11_ClientMessage;
    ev.window = (Window)canvas_info.canvas[window_id].window;
    ev.message_type = _canvas_data[window_id].x11_net_wm_moveresize;
    ev.format = 32;
    ev.data.l[0] = x_root;
    ev.data.l[1] = y_root;
    ev.data.l[2] = action;
    ev.data.l[3] = 1;
    ev.data.l[4] = 1;

    x11.XUngrabPointer(x11.display, X11_CurrentTime);
    x11.XSendEvent(x11.display, x11.XDefaultRootWindow(x11.display), false, (1L << 20) | (1L << 19), (XEvent *)&ev);
    x11.XFlush(x11.display);
    CANVAS_RETURN_VOID();
}

int _canvas_window(int64_t x, int64_t y, int64_t width, int64_t height, const char *title)
{
    CANVAS_ENTER_FUNC();

    CANVAS_ASSERT(width > 0);
    CANVAS_ASSERT(height > 0);

    canvas_startup();

    canvas_window_handle window = 0;

    int window_id = _canvas_get_free();
    if (window_id < 0)
    {
        CANVAS_RETURN(window_id);
    }
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);

    canvas_info.canvas[window_id] = (canvas_type){0};
    _canvas_data[window_id] = (canvas_data){0};

#if CANVAS_VALIDATION >= 5
    // Restore canaries after zeroing
    canvas_info.canvas[window_id]._canary_head = CANVAS_CANARY_HEAD;
    canvas_info.canvas[window_id]._canary_tail = CANVAS_CANARY_TAIL;
#endif

    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!x11.display)
        {
            CANVAS_RETURN_ERR(CANVAS_ERR_GET_DISPLAY, "no display connection for window creation\\n");
        }

        window = (canvas_window_handle)x11.XCreateSimpleWindow(
            x11.display,
            x11.XDefaultRootWindow(x11.display),
            x, y, width, height, 0,
            x11.XBlackPixel(x11.display, 0),
            x11.XWhitePixel(x11.display, 0));

        if (!window)
        {
            CANVAS_RETURN_ERR(CANVAS_ERR_GET_WINDOW, "create x11 window\\n");
        }

        _canvas_data[window_id].client_set = true;

        _canvas_data[window_id].x11_wm_protocols = x11.XInternAtom(x11.display, "WM_PROTOCOLS", 0);
        _canvas_data[window_id].x11_wm_delete_window = x11.XInternAtom(x11.display, "WM_DELETE_WINDOW", 0);
        _canvas_data[window_id].x11_wm_state = x11.XInternAtom(x11.display, "WM_STATE", 0);
        _canvas_data[window_id].x11_net_wm_state = x11.XInternAtom(x11.display, "_NET_WM_STATE", 0);
        _canvas_data[window_id].x11_net_wm_state_maximized_horz = x11.XInternAtom(x11.display, "_NET_WM_STATE_MAXIMIZED_HORZ", 0);
        _canvas_data[window_id].x11_net_wm_state_maximized_vert = x11.XInternAtom(x11.display, "_NET_WM_STATE_MAXIMIZED_VERT", 0);
        _canvas_data[window_id].x11_net_wm_state_fullscreen = x11.XInternAtom(x11.display, "_NET_WM_STATE_FULLSCREEN", 0);
        _canvas_data[window_id].x11_motif_wm_hints = x11.XInternAtom(x11.display, "_MOTIF_WM_HINTS", 0);
        _canvas_data[window_id].x11_net_wm_moveresize = x11.XInternAtom(x11.display, "_NET_WM_MOVERESIZE", 0);
        _canvas_data[window_id].x11_atoms_initialized = true;

        MotifWmHints hints = {0};
        hints.flags = MWM_HINTS_DECORATIONS;
        hints.decorations = 0;

        x11.XChangeProperty(x11.display, (Window)window,
                            _canvas_data[window_id].x11_motif_wm_hints,
                            _canvas_data[window_id].x11_motif_wm_hints,
                            32, PropModeReplace,
                            (unsigned char *)&hints, 5);

        Atom window_type = x11.XInternAtom(x11.display, "_NET_WM_WINDOW_TYPE", 0);
        Atom window_type_normal = x11.XInternAtom(x11.display, "_NET_WM_WINDOW_TYPE_NORMAL", 0);
        x11.XChangeProperty(x11.display, (Window)window, window_type, XA_ATOM, 32, PropModeReplace, (unsigned char *)&window_type_normal, 1);

        x11.XSetWMProtocols(x11.display, (Window)window, &_canvas_data[window_id].x11_wm_delete_window, 1);

        long event_mask = (1L << 0) |  // KeyPressMask
                          (1L << 1) |  // KeyReleaseMask
                          (1L << 2) |  // ButtonPressMask
                          (1L << 3) |  // ButtonReleaseMask
                          (1L << 5) |  // PointerMotionMask
                          (1L << 6) |  // PointerMotionHintMask
                          (1L << 15) | // ExposureMask
                          (1L << 17) | // StructureNotifyMask
                          (1L << 19) | // FocusChangeMask
                          (1L << 22);  // PropertyChangeMask

        x11.XSelectInput(x11.display, (Window)window, event_mask);

        x11.XMapWindow(x11.display, (Window)window);
    }

    canvas_info.canvas[window_id].window = (canvas_window_handle)window;
    canvas_info.canvas[window_id].resize = false;
    canvas_info.canvas[window_id].index = window_id;
    canvas_info.canvas[window_id].titlebar = false;
    canvas_info.canvas[window_id].minimized = false;
    canvas_info.canvas[window_id].maximized = false;
    canvas_info.canvas[window_id].fullscreen = false;
    canvas_info.canvas[window_id]._valid = true;

    CANVAS_RETURN(window_id);
}

int _canvas_init_wayland()
{
    wl.library = canvas_library_load(canvas_wayland_library_names, 2);

    if (!wl.library)
    {
        CANVAS_ERR("libwayland-client.so.0 or libwayland-client.so not found");
        return CANVAS_ERR_LOAD_LIBRARY;
    }

    LOAD_WL(wl_display_connect);
    wl.display = wl.wl_display_connect(NULL);

    if (!wl.display)
    {
        CANVAS_ERR("open wayland display\n");
        dlclose(wl.library);
        return CANVAS_ERR_GET_DISPLAY;
    }

    LOAD_WL(wl_display_get_registry);
    wl.registry = wl.wl_display_get_registry(wl.display);

    if (!wl.registry)
    {
        CANVAS_ERR("load wayland registry\n");
        dlclose(wl.library);
        return CANVAS_ERR_GET_DISPLAY;
    }

    LOAD_WL(wl_registry_add_listener);
    int result = wl.wl_registry_add_listener(wl.registry, NULL, &wl);

    if (!result)
    {
        CANVAS_ERR("add wayland registry\n");
        dlclose(wl.library);
        return CANVAS_ERR_GET_DISPLAY;
    }

    LOAD_WL(wl_display_disconnect);
    LOAD_WL(wl_display_dispatch);
    LOAD_WL(wl_display_roundtrip);
    LOAD_WL(wl_registry_bind);
    LOAD_WL(wl_compositor_create_surface);
    LOAD_WL(wl_surface_destroy);
    LOAD_WL(wl_compositor_destroy);
    LOAD_WL(wl_proxy_destroy);

    _canvas_using_wayland = true;

    return CANVAS_OK;
}

static unsigned int _canvas_get_x11_cursor_id(canvas_cursor_type cursor)
{
    switch (cursor)
    {
    case CANVAS_CURSOR_ARROW:
        return 2; // XC_arrow
    case CANVAS_CURSOR_TEXT:
        return 152; // XC_xterm
    case CANVAS_CURSOR_CROSSHAIR:
        return 34; // XC_crosshair
    case CANVAS_CURSOR_HAND:
        return 58; // XC_hand2
    case CANVAS_CURSOR_SIZE_NS:
        return 116; // XC_sb_v_double_arrow
    case CANVAS_CURSOR_SIZE_EW:
        return 108; // XC_sb_h_double_arrow
    case CANVAS_CURSOR_SIZE_NESW:
        return 52;
    case CANVAS_CURSOR_SIZE_NWSE:
        return 52;
    case CANVAS_CURSOR_SIZE_ALL:
        return 52; // XC_fleur
    case CANVAS_CURSOR_NOT_ALLOWED:
        return 0; // XC_X_cursor
    case CANVAS_CURSOR_WAIT:
        return 150; // XC_watch
    default:
        return 2; // XC_arrow
    }
}

int canvas_cursor(int window_id, canvas_cursor_type cursor)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);

    if (_canvas_using_wayland)
    {
        CANVAS_RETURN(CANVAS_OK);
    }

    canvas_info.canvas[window_id].cursor = cursor;
    canvas_info.canvas[window_id].active_cursor = cursor;

    if (!x11.cursors_loaded)
    {
        for (int i = 0; i < 11; i++)
        {
            unsigned int id = _canvas_get_x11_cursor_id((canvas_cursor_type)i);
            x11.cursors[i] = (unsigned long)x11.XCreateFontCursor(x11.display, id);
            if (!x11.cursors[i])
            {
                CANVAS_ERR("failed to create cursor %d\n", i);
                x11.cursors[i] = (unsigned long)x11.XCreateFontCursor(x11.display, 2);
            }
        }
        x11.cursors_loaded = true;
    }

    if (x11.cursors[cursor])
    {
        x11.XDefineCursor(x11.display, (Window)canvas_info.canvas[window_id].window, x11.cursors[cursor]);
        x11.XFlush(x11.display);
    }

    CANVAS_RETURN(CANVAS_OK);
}

static void _canvas_set_active_cursor(int window_id, canvas_cursor_type cursor)
{
    CANVAS_ENTER_FUNC();
    if (canvas_info.canvas[window_id].active_cursor == cursor)
        CANVAS_RETURN_VOID();

    canvas_info.canvas[window_id].active_cursor = cursor;

    if (!x11.cursors_loaded)
    {
        for (int i = 0; i < 11; i++)
        {
            unsigned int id = _canvas_get_x11_cursor_id((canvas_cursor_type)i);
            x11.cursors[i] = (unsigned long)x11.XCreateFontCursor(x11.display, id);
        }
        x11.cursors_loaded = true;
    }

    x11.XDefineCursor(x11.display, (Window)canvas_info.canvas[window_id].window, x11.cursors[cursor]);
    x11.XFlush(x11.display);
    CANVAS_RETURN_VOID();
}

static int x11_error_handler(Display *display, XErrorEvent *error)
{
    char error_text[256];
    x11.XGetErrorText(display, error->error_code, error_text, sizeof(error_text));

    CANVAS_ERR("X11 Error: %s\n", error_text);
    CANVAS_ERR("  Request code: %d\n", error->request_code);
    CANVAS_ERR("  Minor code: %d\n", error->minor_code);
    CANVAS_ERR("  Resource ID: %lu\n", error->resourceid);

    return 0;
}

int _canvas_init_x11()
{
    CANVAS_ENTER_FUNC();

    x11.library = canvas_library_load(canvas_x11_library_names, 2);

    if (!x11.library)
    {
        CANVAS_ERR("libX11.so.6 or libX11.so not found");
        return CANVAS_ERR_LOAD_LIBRARY;
    }

    LOAD_X11(XOpenDisplay);

    x11.display = x11.XOpenDisplay(NULL);

    if (!x11.display)
    {
        dlclose(x11.library);
        CANVAS_ERR("open x11 display\\n");
        return CANVAS_ERR_GET_DISPLAY;
    }

    LOAD_X11(XSetErrorHandler);
    LOAD_X11(XGetErrorText);

    x11.XSetErrorHandler(x11_error_handler);

    LOAD_X11(XCloseDisplay);
    LOAD_X11(XCreateSimpleWindow);
    LOAD_X11(XDestroyWindow);
    LOAD_X11(XMapWindow);
    LOAD_X11(XStoreName);
    LOAD_X11(XSelectInput);
    LOAD_X11(XNextEvent);
    LOAD_X11(XSendEvent);
    LOAD_X11(XPending);
    LOAD_X11(XRootWindow);
    LOAD_X11(XFlush);
    LOAD_X11(XMoveResizeWindow);
    LOAD_X11(XDisplayWidth);
    LOAD_X11(XDisplayHeight);
    LOAD_X11(XInternAtom);
    LOAD_X11(XIconifyWindow);
    LOAD_X11(XSetWMProtocols);
    LOAD_X11(XBlackPixel);
    LOAD_X11(XWhitePixel);
    LOAD_X11(XDefaultRootWindow);
    LOAD_X11(XDefaultScreen);
    LOAD_X11(XChangeProperty);
    LOAD_X11(XGetWindowAttributes);
    LOAD_X11(XGetWindowProperty);
    LOAD_X11(XGrabPointer);
    LOAD_X11(XUngrabPointer);
    LOAD_X11(XMoveWindow);
    LOAD_X11(XCreateFontCursor);
    LOAD_X11(XDefineCursor);
    LOAD_X11(XFree);
    LOAD_X11(XQueryPointer);
    LOAD_X11(XTranslateCoordinates);
    LOAD_X11(XFreeCursor);

    x11.internal_atom = x11.XInternAtom(x11.display, "_CANVAS_INTERNAL", false);

    xrandr.library = canvas_library_load(canvas_xrandr_library_names, 2);

    if (!xrandr.library)
    {
        CANVAS_WARN("libXrandr.so.2 or libXrandr.so not found");
        return CANVAS_OK;
    }

    xrandr.XRRGetScreenResourcesCurrent = canvas_library_symbol(xrandr.library, "XRRGetScreenResourcesCurrent");
    xrandr.XRRFreeScreenResources = canvas_library_symbol(xrandr.library, "XRRFreeScreenResources");
    xrandr.XRRGetCrtcInfo = canvas_library_symbol(xrandr.library, "XRRGetCrtcInfo");
    xrandr.XRRFreeCrtcInfo = canvas_library_symbol(xrandr.library, "XRRFreeCrtcInfo");
    xrandr.XRRGetOutputInfo = canvas_library_symbol(xrandr.library, "XRRGetOutputInfo");
    xrandr.XRRFreeOutputInfo = canvas_library_symbol(xrandr.library, "XRRFreeOutputInfo");
    xrandr.XRRGetOutputPrimary = canvas_library_symbol(xrandr.library, "XRRGetOutputPrimary");

    if (!xrandr.XRRGetScreenResourcesCurrent || !xrandr.XRRFreeScreenResources || !xrandr.XRRGetCrtcInfo || !xrandr.XRRFreeCrtcInfo || !xrandr.XRRGetOutputInfo || !xrandr.XRRFreeOutputInfo || !xrandr.XRRGetOutputPrimary)
    {
        dlclose(xrandr.library);
        CANVAS_WARN("failed to load Xrandr symbols");
        return CANVAS_OK;
    }

    _canvas_using_wayland = false;

    CANVAS_INFO("X11 initialized successfully\\n");
    return CANVAS_OK;
}

int _canvas_platform()
{
    CANVAS_ENTER_FUNC();
    if (canvas_info.init)
    {
        CANVAS_RETURN(CANVAS_OK);
    }

    //     int load_result = _canvas_init_wayland();

    if (!_canvas_using_wayland && _canvas_init_x11() < 0)
    {
        CANVAS_RETURN(CANVAS_ERR_GET_DISPLAY);
    }

    CANVAS_RETURN(CANVAS_OK);
}

static canvas_cursor_type _canvas_get_resize_cursor(int action)
{
    CANVAS_ENTER_FUNC();
    canvas_cursor_type result;
    switch (action)
    {
    case _NET_WM_MOVERESIZE_SIZE_TOPLEFT:
        result = CANVAS_CURSOR_SIZE_NESW;
        break;
    case _NET_WM_MOVERESIZE_SIZE_TOPRIGHT:
        result = CANVAS_CURSOR_SIZE_NWSE;
        break;
    case _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT:
        result = CANVAS_CURSOR_SIZE_NESW;
        break;
    case _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT:
        result = CANVAS_CURSOR_SIZE_NWSE;
        break;
    case _NET_WM_MOVERESIZE_SIZE_TOP:
    case _NET_WM_MOVERESIZE_SIZE_BOTTOM:
        result = CANVAS_CURSOR_SIZE_NS;
        break;
    case _NET_WM_MOVERESIZE_SIZE_LEFT:
    case _NET_WM_MOVERESIZE_SIZE_RIGHT:
        result = CANVAS_CURSOR_SIZE_EW;
        break;
    default:
        result = CANVAS_CURSOR_ARROW;
        break;
    }
    CANVAS_RETURN(result);
}

static bool _canvas_is_window_maximized(int window_id)
{
    CANVAS_ENTER_FUNC();
    Window window = (Window)canvas_info.canvas[window_id].window;

    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop_data = NULL;

    Atom net_wm_state = _canvas_data[window_id].x11_net_wm_state;

    int result = x11.XGetWindowProperty(
        x11.display, window,
        net_wm_state,
        0, 1024, false, XA_ATOM,
        &actual_type, &actual_format,
        &nitems, &bytes_after,
        &prop_data);

    if (result != 0 || !prop_data)
        CANVAS_RETURN(false);

    Atom *atoms = (Atom *)prop_data;
    bool horz_maximized = false;
    bool vert_maximized = false;

    for (unsigned long i = 0; i < nitems; i++)
    {
        if (atoms[i] == _canvas_data[window_id].x11_net_wm_state_maximized_horz)
            horz_maximized = true;
        if (atoms[i] == _canvas_data[window_id].x11_net_wm_state_maximized_vert)
            vert_maximized = true;
    }

    x11.XFree(prop_data);

    CANVAS_RETURN(horz_maximized && vert_maximized);
}

int _canvas_update()
{
    CANVAS_ENTER_FUNC();
#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT(canvas_info._canary_head == CANVAS_CANARY_HEAD);
    CANVAS_ASSERT(canvas_info._canary_tail == CANVAS_CANARY_TAIL);
#endif

    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!x11.display)
        {
            CANVAS_VERBOSE("no display connection for update\n");
            CANVAS_RETURN(CANVAS_ERR_GET_DISPLAY);
        }

        canvas_pointer *p = canvas_get_primary_pointer(0);
        CANVAS_ASSERT_NOT_NULL(p);

        Window root_return, child_return;
        int root_x, root_y, win_x, win_y;
        unsigned int mask_return;

        if (x11.XQueryPointer(x11.display, x11.XDefaultRootWindow(x11.display),
                              &root_return, &child_return,
                              &root_x, &root_y, &win_x, &win_y, &mask_return))
        {
            p->display = 0;
            for (int d = 0; d < canvas_info.display_count; d++)
            {
                if (root_x >= canvas_info.display[d].x &&
                    root_x < canvas_info.display[d].x + canvas_info.display[d].width &&
                    root_y >= canvas_info.display[d].y &&
                    root_y < canvas_info.display[d].y + canvas_info.display[d].height)
                {
                    p->display = d;
                    p->screen_x = root_x - canvas_info.display[d].x;
                    p->screen_y = root_y - canvas_info.display[d].y;
                    break;
                }
            }

            uint32_t old_buttons = p->buttons;
            uint32_t new_buttons = 0;

            if (mask_return & (1 << 8))
                new_buttons |= CANVAS_BUTTON_LEFT;
            if (mask_return & (1 << 9))
                new_buttons |= CANVAS_BUTTON_MIDDLE;
            if (mask_return & (1 << 10))
                new_buttons |= CANVAS_BUTTON_RIGHT;
            if (mask_return & (1 << 11))
                new_buttons |= CANVAS_BUTTON_X1;
            if (mask_return & (1 << 12))
                new_buttons |= CANVAS_BUTTON_X2;

            p->buttons_pressed = new_buttons & ~old_buttons;
            p->buttons_released = old_buttons & ~new_buttons;
            p->buttons = new_buttons;

            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            double timestamp = (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;

            p->_samples[p->_sample_index].x = p->screen_x;
            p->_samples[p->_sample_index].y = p->screen_y;
            p->_samples[p->_sample_index].time = timestamp;
            p->_sample_index = (p->_sample_index + 1) % CANVAS_POINTER_SAMPLE_FRAMES;

            bool found_window = false;

            for (int i = MAX_CANVAS - 1; i >= 0; i--)
            {
                if (!canvas_info.canvas[i]._valid || !canvas_info.canvas[i].window)
                    continue;

                XWindowAttributes attr;
                if (x11.XGetWindowAttributes(x11.display, (Window)canvas_info.canvas[i].window, &attr))
                {
                    Window child;
                    int wx, wy;
                    x11.XTranslateCoordinates(x11.display, (Window)canvas_info.canvas[i].window,
                                              attr.root, 0, 0, &wx, &wy, &child);

                    bool is_inside = (root_x >= wx && root_x < wx + attr.width &&
                                      root_y >= wy && root_y < wy + attr.height);

                    if (is_inside && !found_window)
                    {
                        found_window = true;

                        p->window_id = i;
                        p->inside_window = true;
                        p->x = root_x - wx;
                        p->y = root_y - wy;

                        if (!canvas_info.canvas[i].maximized)
                        {
                            int action = _canvas_get_resize_edge_action(i, p->x, p->y);
                            if (action >= 0)
                            {
                                canvas_cursor_type resize_cursor = _canvas_get_resize_cursor(action);
                                _canvas_set_active_cursor(i, resize_cursor);
                            }
                            else if (p->y < 30)
                            {
                                _canvas_set_active_cursor(i, CANVAS_CURSOR_ARROW);
                            }
                            else
                            {
                                _canvas_set_active_cursor(i, canvas_info.canvas[i].cursor);
                            }
                        }
                        else
                        {
                            if (p->y < 30)
                            {
                                _canvas_set_active_cursor(i, CANVAS_CURSOR_ARROW);
                            }
                            else
                            {
                                _canvas_set_active_cursor(i, canvas_info.canvas[i].cursor);
                            }
                        }
                    }
                }
            }

            if (!found_window)
            {
                p->inside_window = false;
                p->x = 0;
                p->y = 0;
            }
        }

        XEvent event;
        while (x11.XPending(x11.display))
        {
            x11.XNextEvent(x11.display, &event);

            int window_id = -1;
            if (event.type >= 2 && event.type <= 35)
            {
                _x11_event *base_event = (_x11_event *)&event;
                window_id = _canvas_window_index((void *)base_event->window);
            }

            if (window_id < 0)
                continue;

            switch (event.type)
            {
            case X11_ConfigureNotify:
            {
                XConfigureEvent *xce = (XConfigureEvent *)&event;

                if (_canvas_data[window_id].client_set)
                {
                    _canvas_data[window_id].client_set = false;
                    break;
                }

                if (xce->send_event)
                    break;

                canvas_info.canvas[window_id].x = xce->x;
                canvas_info.canvas[window_id].y = xce->y;

                bool size_changed = (canvas_info.canvas[window_id].width != xce->width ||
                                     canvas_info.canvas[window_id].height != xce->height);

                if (size_changed)
                {
                    canvas_info.canvas[window_id].width = xce->width;
                    canvas_info.canvas[window_id].height = xce->height;
                    canvas_info.canvas[window_id].resize = true;
                    canvas_info.canvas[window_id].os_resized = true;

                    canvas_vulkan_window *vk_win = &vk_windows[window_id];
                    if (vk_win->initialized)
                    {
                        vk_win->needs_resize = true;

                        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                        {
                            if (vk_win->in_flight_fences[i] != VK_NULL_HANDLE)
                            {
                                vk_info.vkWaitForFences(vk_info.device, 1,
                                                        &vk_win->in_flight_fences[i], VK_TRUE, UINT64_MAX);
                            }
                        }

                        vk_recreate_swapchain(window_id);
                    }
                }

                break;
            }

            case X11_ButtonPress:
            {
                XButtonEvent *xbe = (XButtonEvent *)&event;

                canvas_pointer *p = canvas_get_primary_pointer(window_id);

                if (xbe->button == 4)
                {
                    p->scroll_y = 1.0f;
                    break;
                }
                if (xbe->button == 5)
                {
                    p->scroll_y = -1.0f;
                    break;
                }
                if (xbe->button == 6)
                {
                    p->scroll_x = -1.0f;
                    break;
                }
                if (xbe->button == 7)
                {
                    p->scroll_x = 1.0f;
                    break;
                }

                if (xbe->button == 1)
                {
                    p->window_id = window_id;

                    int time_diff = xbe->time - _canvas_data[window_id].last_button_press_time;
                    int dx = xbe->x - _canvas_data[window_id].last_button_press_x;
                    int dy = xbe->y - _canvas_data[window_id].last_button_press_y;

                    if (time_diff < 400 && dx * dx + dy * dy < 25 && xbe->y < 30)
                    {
                        if (canvas_info.canvas[window_id].maximized)
                            canvas_restore(window_id);
                        else
                            canvas_maximize(window_id);
                    }
                    else
                    {
                        if (!canvas_info.canvas[window_id].maximized)
                        {
                            int action = _canvas_get_resize_edge_action(window_id, xbe->x, xbe->y);

                            if (action >= 0)
                            {
                                canvas_info.canvas[window_id].os_resized = true;
                                _canvas_start_wm_move_resize(window_id, xbe->x_root, xbe->y_root, action);
                            }
                            else if (xbe->y < 30)
                            {
                                canvas_info.canvas[window_id].os_moved = true;
                                _canvas_start_wm_move_resize(window_id, xbe->x_root, xbe->y_root, _NET_WM_MOVERESIZE_MOVE);
                            }
                        }
                        else if (xbe->y < 30)
                        {
                            canvas_info.canvas[window_id].os_moved = true;
                            _canvas_start_wm_move_resize(window_id, xbe->x_root, xbe->y_root, _NET_WM_MOVERESIZE_MOVE);
                        }
                    }

                    _canvas_data[window_id].last_button_press_time = xbe->time;
                    _canvas_data[window_id].last_button_press_x = xbe->x;
                    _canvas_data[window_id].last_button_press_y = xbe->y;
                }

                break;
            }

            case X11_PropertyNotify:
            {
                XPropertyEvent *xpe = (XPropertyEvent *)&event;

                if (xpe->atom == _canvas_data[window_id].x11_net_wm_state)
                {
                    bool was_maximized = canvas_info.canvas[window_id].maximized;
                    bool is_maximized = _canvas_is_window_maximized(window_id);

                    canvas_info.canvas[window_id].maximized = is_maximized;

                    if (was_maximized != is_maximized)
                        canvas_info.canvas[window_id].active_cursor = CANVAS_CURSOR_ARROW;
                }
                break;
            }

            case X11_ClientMessage:
            {
                XClientMessageEvent *cm = (XClientMessageEvent *)&event;

                if ((Atom)cm->data.l[0] == _canvas_data[window_id].x11_wm_delete_window)
                    canvas_info.canvas[window_id].close = true;

                break;
            }

            case X11_UnmapNotify:
            {
                canvas_info.canvas[window_id].minimized = true;
                canvas_info.canvas[window_id].maximized = false;
                break;
            }

            case 2:
            {
                XKeyEvent *xke = (XKeyEvent *)&event;
                unsigned int keycode = xke->keycode;

                if (keycode >= 8 && keycode < 256)
                {
                    keycode -= 8;
                    int hid = x11_to_hid[keycode];
                    if (hid > 0 && hid < 256 && !canvas_keyboard.keys[hid])
                    {
                        canvas_keyboard.keys[hid] = true;
                        canvas_keyboard.keys_pressed[hid] = true;
                    }
                }
                break;
            }

            case 3:
            {
                XKeyEvent *xke = (XKeyEvent *)&event;
                unsigned int keycode = xke->keycode;

                if (keycode >= 8 && keycode < 256)
                {
                    keycode -= 8;
                    int hid = x11_to_hid[keycode];
                    if (hid > 0 && hid < 256 && canvas_keyboard.keys[hid])
                    {
                        canvas_keyboard.keys[hid] = false;
                        canvas_keyboard.keys_released[hid] = true;
                    }
                }
                break;
            }

            case X11_MapNotify:
            {
                canvas_info.canvas[window_id].minimized = false;
                break;
            }
            }
        }
    }

    // Render all windows
    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (!canvas_info.canvas[i]._valid || !vk_windows[i].initialized)
            continue;

        vk_draw_frame(i);
    }

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_gpu_init(void)
{
    CANVAS_ENTER_FUNC();
    if (canvas_info.init_gpu)
    {
        CANVAS_RETURN(CANVAS_OK);
    }

    int result = canvas_backend_vulkan_init();
    if (result != CANVAS_OK)
    {
        CANVAS_RETURN(result);
    }

    canvas_info.init_gpu = 1;
    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_gpu_new_window(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];
    memset(vk_win, 0, sizeof(canvas_vulkan_window));

    int result;

    result = vk_create_surface(window_id, &vk_win->surface);
    if (result != CANVAS_OK)
    {
        CANVAS_RETURN_ERR(result, "failed to create surface for window %d\n", window_id);
    }

    if (!vk_info.device)
    {
        VkResult vk_result = vk_select_physical_device(vk_win->surface);
        if (vk_result != VK_SUCCESS)
        {
            CANVAS_ERR("failed to select physical device\n");
            vk_info.vkDestroySurfaceKHR(vk_info.instance, vk_win->surface, NULL);
            CANVAS_RETURN(CANVAS_ERR_GET_GPU);
        }

        result = vk_create_logical_device();
        if (result != CANVAS_OK)
        {
            vk_info.vkDestroySurfaceKHR(vk_info.instance, vk_win->surface, NULL);
            CANVAS_RETURN(result);
        }

        result = vk_load_device_functions();
        if (result != CANVAS_OK)
        {
            vk_info.vkDestroyDevice(vk_info.device, NULL);
            vk_info.vkDestroySurfaceKHR(vk_info.instance, vk_win->surface, NULL);
            CANVAS_RETURN(result);
        }
    }

    result = vk_create_swapchain(window_id);
    if (result != CANVAS_OK)
        goto cleanup;

    result = vk_create_render_pass(window_id);
    if (result != CANVAS_OK)
        goto cleanup;

    result = vk_create_framebuffers(window_id);
    if (result != CANVAS_OK)
        goto cleanup;

    result = vk_create_command_pool(window_id);
    if (result != CANVAS_OK)
        goto cleanup;

    result = vk_create_command_buffers(window_id);
    if (result != CANVAS_OK)
        goto cleanup;

    result = vk_create_sync_objects(window_id);
    if (result != CANVAS_OK)
        goto cleanup;

    vk_win->initialized = true;
    vk_win->current_frame = 0;

    CANVAS_VERBOSE("vulkan setup complete for window %d\n", window_id);
    CANVAS_RETURN(CANVAS_OK);

cleanup:
    vk_cleanup_window(window_id);
    CANVAS_RETURN(result);
}

int _canvas_window_resize(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_BOUNDS(window_id);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    if (!vk_win->initialized)
    {
        CANVAS_RETURN(CANVAS_OK);
    }

    vk_win->needs_resize = true;
    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_post_update()
{
    CANVAS_ENTER_FUNC();
    if (_canvas_using_wayland)
    {
    }
    else
    {
        x11.XFlush(x11.display);
    }

    CANVAS_RETURN(CANVAS_OK);
}

int _canvas_exit()
{
    CANVAS_ENTER_FUNC();
    if (vk_info.library)
        vk_cleanup();

    if (x11.cursors_loaded)
    {
        for (int i = 0; i < 11; i++)
        {
            if (x11.cursors[i])
                x11.XFreeCursor(x11.display, x11.cursors[i]);
        }
    }

    if (x11.display)
        x11.XCloseDisplay(x11.display);

    if (x11.library)
        dlclose(x11.library);

    if (xrandr.library)
        dlclose(xrandr.library);

    CANVAS_RETURN(CANVAS_OK);
}

double canvas_get_time(canvas_time_data *time)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT(time != NULL);

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    if (ts.tv_sec < 0 || ts.tv_nsec < 0)
    {
        CANVAS_RETURN(0.0);
    }

    uint64_t now = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;

    double result = (double)(now - time->start) / 1e9;
    CANVAS_RETURN(result);
}

void canvas_sleep(double seconds)
{
    CANVAS_ENTER_FUNC();

    struct timespec ts;
    ts.tv_sec = (time_t)seconds;
    ts.tv_nsec = (long)((seconds - (double)ts.tv_sec) * 1e9);
    clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);

    CANVAS_RETURN_VOID();
}

void canvas_time_init(canvas_time_data *time)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT(time != NULL);
    time->frame = 0;
    time->frame_index = 0;
    time->accumulator = 0;
    time->alpha = 0;
    time->delta = 0;
    time->fps = 0;
    time->raw_delta = 0;

    for (int i = 0; i < 60; i++)
        time->times[i] = 0.0;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    time->start = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    time->current = canvas_get_time(time);
    time->last = time->current;
    CANVAS_RETURN_VOID();
}

#endif // _linux_

bool canvas_pointer_down(canvas_pointer *p, canvas_pointer_button btn)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(p);
#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT_NOT_POISONED(p, sizeof(canvas_pointer));
    CANVAS_ASSERT(p->_canary_head == CANVAS_CANARY_HEAD);
    CANVAS_ASSERT(p->_canary_tail == CANVAS_CANARY_TAIL);
#endif
    CANVAS_RETURN((p->buttons & btn) != 0);
}

bool canvas_pointer_pressed(canvas_pointer *p, canvas_pointer_button btn)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(p);
#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT_NOT_POISONED(p, sizeof(canvas_pointer));
    CANVAS_ASSERT(p->_canary_head == CANVAS_CANARY_HEAD);
    CANVAS_ASSERT(p->_canary_tail == CANVAS_CANARY_TAIL);
#endif
    CANVAS_RETURN((p->buttons_pressed & btn) != 0);
}

bool canvas_pointer_released(canvas_pointer *p, canvas_pointer_button btn)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(p);
#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT_NOT_POISONED(p, sizeof(canvas_pointer));
    CANVAS_ASSERT(p->_canary_head == CANVAS_CANARY_HEAD);
    CANVAS_ASSERT(p->_canary_tail == CANVAS_CANARY_TAIL);
#endif
    CANVAS_RETURN((p->buttons_released & btn) != 0);
}

canvas_pointer *canvas_get_pointer(int id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_RANGE(id, 0, CANVAS_POINTER_BUDGET - 1);
    if (id < 0 || id >= CANVAS_POINTER_BUDGET)
        CANVAS_RETURN(NULL);

    canvas_pointer *p = canvas_info.pointers[id].active ? &canvas_info.pointers[id] : NULL;
    if (p)
    {
        CANVAS_ASSERT(p->active);
        CANVAS_ASSERT_RANGE(p->_sample_index, 0, CANVAS_POINTER_SAMPLE_FRAMES - 1);
#if CANVAS_VALIDATION >= 5
        CANVAS_ASSERT_NOT_POISONED(p, sizeof(canvas_pointer));
        CANVAS_ASSERT(p->_canary_head == CANVAS_CANARY_HEAD);
        CANVAS_ASSERT(p->_canary_tail == CANVAS_CANARY_TAIL);
#endif
    }
    CANVAS_RETURN(p);
}

float canvas_pointer_velocity(canvas_pointer *p)
{
    CANVAS_ENTER_FUNC();
    if (!p)
        CANVAS_RETURN(0.0f);

#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT_NOT_POISONED(p, sizeof(canvas_pointer));
    CANVAS_ASSERT(p->_canary_head == CANVAS_CANARY_HEAD);
    CANVAS_ASSERT(p->_canary_tail == CANVAS_CANARY_TAIL);
    CANVAS_ASSERT_RANGE(p->_sample_index, 0, CANVAS_POINTER_SAMPLE_FRAMES - 1);
#endif

    int newest = (p->_sample_index - 1 + CANVAS_POINTER_SAMPLE_FRAMES) % CANVAS_POINTER_SAMPLE_FRAMES;
    int oldest = p->_sample_index;

    canvas_pointer_sample *s_new = &p->_samples[newest];
    canvas_pointer_sample *s_old = &p->_samples[oldest];

    double dt = s_new->time - s_old->time;

    if (dt < 0.001)
        CANVAS_RETURN(0.0f);

    int64_t dx = s_new->x - s_old->x;
    int64_t dy = s_new->y - s_old->y;

    CANVAS_RETURN((float)(sqrt((double)(dx * dx + dy * dy)) / dt));
}

void canvas_pointer_delta(canvas_pointer *p, int64_t *dx, int64_t *dy)
{
    CANVAS_ENTER_FUNC();
    if (dx)
        *dx = 0;
    if (dy)
        *dy = 0;

    if (!p || !dx || !dy)
        CANVAS_RETURN_VOID();

#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT_NOT_POISONED(p, sizeof(canvas_pointer));
    CANVAS_ASSERT(p->_canary_head == CANVAS_CANARY_HEAD);
    CANVAS_ASSERT(p->_canary_tail == CANVAS_CANARY_TAIL);
    CANVAS_ASSERT_RANGE(p->_sample_index, 0, CANVAS_POINTER_SAMPLE_FRAMES - 1);
#endif

    int newest = (p->_sample_index - 1 + CANVAS_POINTER_SAMPLE_FRAMES) % CANVAS_POINTER_SAMPLE_FRAMES;
    int prev = (newest - 1 + CANVAS_POINTER_SAMPLE_FRAMES) % CANVAS_POINTER_SAMPLE_FRAMES;

    *dx = p->_samples[newest].x - p->_samples[prev].x;
    *dy = p->_samples[newest].y - p->_samples[prev].y;
    CANVAS_RETURN_VOID();
}

int canvas_get_active_pointers(canvas_pointer **out)
{
    CANVAS_ENTER_FUNC();
    CANVAS_PARANOID_CHECK();
    if (!out)
        CANVAS_RETURN(0);

    int count = 0;
    for (int i = 0; i < canvas_info.pointer_count; i++)
    {
        CANVAS_ASSERT_RANGE(i, 0, CANVAS_POINTER_BUDGET - 1);
        if (canvas_info.pointers[i].active)
        {
#if CANVAS_VALIDATION >= 5
            CANVAS_ASSERT(canvas_info.pointers[i]._canary_head == CANVAS_CANARY_HEAD);
            CANVAS_ASSERT(canvas_info.pointers[i]._canary_tail == CANVAS_CANARY_TAIL);
#endif
            out[count++] = &canvas_info.pointers[i];
        }
    }
    CANVAS_RETURN(count);
}

float canvas_pointer_direction(canvas_pointer *p)
{
    CANVAS_ENTER_FUNC();
    if (!p)
        CANVAS_RETURN(0.0f);

#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT_NOT_POISONED(p, sizeof(canvas_pointer));
    CANVAS_ASSERT(p->_canary_head == CANVAS_CANARY_HEAD);
    CANVAS_ASSERT(p->_canary_tail == CANVAS_CANARY_TAIL);
    CANVAS_ASSERT_RANGE(p->_sample_index, 0, CANVAS_POINTER_SAMPLE_FRAMES - 1);
#endif

    int newest = (p->_sample_index - 1 + CANVAS_POINTER_SAMPLE_FRAMES) % CANVAS_POINTER_SAMPLE_FRAMES;
    int oldest = p->_sample_index;

    canvas_pointer_sample *s_new = &p->_samples[newest];
    canvas_pointer_sample *s_old = &p->_samples[oldest];

    int64_t dx = s_new->x - s_old->x;
    int64_t dy = s_new->y - s_old->y;

    CANVAS_RETURN(atan2f((float)dy, (float)dx));
}

void canvas_pointer_capture(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_PARANOID_OP("canvas_pointer_capture");
    CANVAS_ASSERT_RANGE(window_id, 0, MAX_CANVAS - 1);

    if (!canvas_info.canvas[window_id]._valid)
        CANVAS_RETURN_VOID();

#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT(canvas_info.canvas[window_id]._canary_head == CANVAS_CANARY_HEAD);
    CANVAS_ASSERT(canvas_info.canvas[window_id]._canary_tail == CANVAS_CANARY_TAIL);
#endif

    CANVAS_ASSERT_NOT_NULL(canvas_info.canvas[window_id].window);

    canvas_pointer *p = canvas_get_primary_pointer(window_id);
    CANVAS_ASSERT_NOT_NULL(p);

    if (!p)
        CANVAS_RETURN_VOID();

    p->captured = true;

#if defined(_WIN32)
    // SetCapture((HWND)canvas_info.canvas[window_id].window);
#elif defined(__linux__)
    if (!_canvas_using_wayland && x11.display)
    {
        x11.XGrabPointer(x11.display,
                         (Window)canvas_info.canvas[window_id].window,
                         true,
                         ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                         X11_GrabModeAsync, X11_GrabModeAsync, None, None, X11_CurrentTime);
    }
#endif
    CANVAS_RETURN_VOID();
}

void canvas_pointer_release()
{
    CANVAS_ENTER_FUNC();
    CANVAS_PARANOID_OP("canvas_pointer_release");
    CANVAS_PARANOID_CHECK();

    for (int i = 0; i < canvas_info.pointer_count; i++)
    {
        CANVAS_ASSERT_RANGE(i, 0, CANVAS_POINTER_BUDGET - 1);
#if CANVAS_VALIDATION >= 5
        CANVAS_ASSERT(canvas_info.pointers[i]._canary_head == CANVAS_CANARY_HEAD);
        CANVAS_ASSERT(canvas_info.pointers[i]._canary_tail == CANVAS_CANARY_TAIL);
#endif
        if (canvas_info.pointers[i].captured)
        {
            canvas_info.pointers[i].captured = false;
        }
    }

#if defined(_WIN32)
    // ReleaseCapture();
#elif defined(__linux__)
    if (!_canvas_using_wayland && x11.display)
    {
        x11.XUngrabPointer(x11.display, X11_CurrentTime);
    }
#endif
    CANVAS_RETURN_VOID();
}

int _canvas_primary_display_index(void)
{
    CANVAS_ENTER_FUNC();
    for (int i = 0; i < canvas_info.display_count; ++i)
        if (canvas_info.display[i].primary)
            CANVAS_RETURN(i);
    CANVAS_RETURN(0);
}

int canvas_startup()
{
    CANVAS_ENTER_FUNC();
    if (canvas_info.init)
        CANVAS_RETURN(CANVAS_OK);

    // Initialize paranoid mode canaries first (before any other initialization)
    _canvas_init_canaries();
    CANVAS_PARANOID_OP("canvas_startup");

    canvas_info.auto_exit = true;
    canvas_info.limit_fps = 240;

    canvas_time_init(&canvas_info.time);

    for (int i = 0; i < MAX_DISPLAYS; ++i)
    {
        canvas_info.display[i] = (canvas_display){0};
    }

    for (int i = 0; i < MAX_CANVAS; ++i)
    {
        canvas_info.canvas[i] = (canvas_type){0};
        _canvas_data[i] = (canvas_data){0};
#if CANVAS_VALIDATION >= 5
        // Re-initialize canaries after zeroing
        canvas_info.canvas[i]._canary_head = CANVAS_CANARY_HEAD;
        canvas_info.canvas[i]._canary_tail = CANVAS_CANARY_TAIL;
#endif
    }

    int result = _canvas_platform();
    if (result != CANVAS_OK)
    {
        CANVAS_RETURN_ERR(result, "platform initialization failed\n");
    }

    result = _canvas_init_displays();
    if (result < 0)
    {
        CANVAS_RETURN_ERR(result, "display initialization failed\n");
    }

    canvas_info.init = true;

    CANVAS_PARANOID_CHECK();
    CANVAS_RETURN(CANVAS_OK);
}

void canvas_main_loop()
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_INITIALIZED();
    CANVAS_PARANOID_OP("canvas_main_loop");

    canvas_time_update(&canvas_info.time);

    _canvas_update();

    bool any_alive = false;
    for (int i = 0; i < MAX_CANVAS; ++i)
    {
        if (!canvas_info.canvas[i]._valid)
            continue;

        any_alive = true;
        CANVAS_ASSERT_NOT_NULL(canvas_info.canvas[i].window);
        CANVAS_ASSERT_RANGE(canvas_info.canvas[i].index, 0, MAX_CANVAS - 1);
        CANVAS_PARANOID_CHECK();

        if (canvas_info.canvas[i].close)
        {
            canvas_close(canvas_info.canvas[i].index);
            continue;
        }

        if (canvas_info.canvas[i].update)
        {
            canvas_info.canvas[i].update(i);
        }
        else if (canvas_info.update_callback)
        {
            canvas_info.update_callback(i);
        }
    }

    _canvas_post_update();

    if (canvas_info.auto_exit && !any_alive)
    {
        CANVAS_DBG("auto exit triggered\\n");
        canvas_info.quit = 1;
    }

    if (!canvas_info.os_timed)
    {
        canvas_limit_fps(&canvas_info.time, canvas_info.limit_fps);
    }

    for (int i = 0; i < canvas_info.pointer_count; i++)
    {
        CANVAS_ASSERT_RANGE(i, 0, CANVAS_POINTER_BUDGET - 1);
        canvas_info.pointers[i].buttons_pressed = 0;
        canvas_info.pointers[i].buttons_released = 0;
        canvas_info.pointers[i].scroll_x = 0;
        canvas_info.pointers[i].scroll_y = 0;
    }

    memset(canvas_keyboard.keys_pressed, 0, sizeof(canvas_keyboard.keys_pressed));
    memset(canvas_keyboard.keys_released, 0, sizeof(canvas_keyboard.keys_released));
    CANVAS_RETURN_VOID();
}

int canvas_exit()
{
    CANVAS_ENTER_FUNC();
    CANVAS_PARANOID_OP("canvas_exit");
    CANVAS_INFO("quitting canvas");
    canvas_info.quit = 1;
    int result = _canvas_exit();
    CANVAS_RETURN(result);
}

int canvas_run(canvas_update_callback default_callback)
{
    CANVAS_ENTER_FUNC();
    CANVAS_PARANOID_OP("canvas_run");
    CANVAS_PARANOID_CHECK();

    if (!canvas_info.init)
    {
        CANVAS_RETURN_ERR(CANVAS_FAIL, "refusing run, initialization failed.");
    }

    canvas_info.update_callback = default_callback;

    while (!canvas_info.quit)
    {
        canvas_info.os_timed = false;
        canvas_main_loop();
    }

    int result = canvas_exit();
    CANVAS_RETURN(result);
}

// display:     -1 = primary display
// x:           -1 = centered
// y:           -1 = centered
// width:       -1 = keep size, window width in pixels
// height:      -1 = keep size, window height in pixels
// title:       NULL = keep title, window title string
int canvas_set(int window_id, int display, int64_t x, int64_t y, int64_t width, int64_t height, const char *title)
{
    CANVAS_ENTER_FUNC();
    CANVAS_PARANOID_OP("canvas_set");
    CANVAS_VALID(window_id);

#if CANVAS_VALIDATION >= 5
    // Validate window canaries
    CANVAS_ASSERT(canvas_info.canvas[window_id]._canary_head == CANVAS_CANARY_HEAD);
    CANVAS_ASSERT(canvas_info.canvas[window_id]._canary_tail == CANVAS_CANARY_TAIL);
    // Sanity check dimensions
    if (width > 0 && width > 32768)
    {
        CANVAS_WARN("suspicious width: %" PRId64 "\\n", width);
    }
    if (height > 0 && height > 32768)
    {
        CANVAS_WARN("suspicious height: %" PRId64 "\\n", height);
    }
#endif

    if (canvas_info.canvas[window_id].fullscreen) // todo: allow changing display in fullscreen
        CANVAS_RETURN(CANVAS_OK);

    if (canvas_info.display_count <= 0)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_GET_DISPLAY, "no displays available\n");
    }

    if (display < 0 || display >= canvas_info.display_count)
    {
        display = _canvas_primary_display_index();
    }

    if (width == 0 || height == 0)
    {
        CANVAS_RETURN_ERR(CANVAS_ERR_INVALID_SIZE, "invalid window size\n");
    }

    canvas_info.canvas[window_id].display = display;

    if (width != -1)
        canvas_info.canvas[window_id].width = width;

    if (height != -1)
        canvas_info.canvas[window_id].height = height;

    canvas_info.canvas[window_id].os_moved = false;
    canvas_info.canvas[window_id].os_resized = false;

    canvas_info.canvas[window_id].x = x;
    canvas_info.canvas[window_id].y = y;

    CANVAS_DISPLAY_BOUNDS(display);

    int target_x = x;
    int target_y = y;

    if (x == -1)
    {
        int display_half = canvas_info.display[display].width / 2;
        int window_half = canvas_info.canvas[window_id].width / 2;
        target_x = display_half > window_half ? display_half - window_half : 0;
    }

    if (y == -1)
    {
        int display_half = canvas_info.display[display].height / 2;
        int window_half = canvas_info.canvas[window_id].height / 2;
        target_y = display_half > window_half ? display_half - window_half : 0;
    }

    if (title)
    {
        size_t len = strnlen(title, MAX_CANVAS_TITLE);
        if (len > 0)
        {
            size_t copy_len = len < MAX_CANVAS_TITLE ? len : MAX_CANVAS_TITLE - 1;
            memcpy(canvas_info.canvas[window_id].title, title, copy_len);
            canvas_info.canvas[window_id].title[copy_len] = '\0';
        }
        else
        {
            canvas_info.canvas[window_id].title[0] = '\0';
        }
    }

    int result = _canvas_set(window_id, display, target_x, target_y, width, height, title);
    CANVAS_RETURN(result);
}

int canvas_window(int64_t x, int64_t y, int64_t width, int64_t height, const char *title)
{
    CANVAS_ENTER_FUNC();
    int result = _canvas_window(x, y, width, height, title);

    if (result < 0)
    {
        CANVAS_RETURN_ERR(result, "window creation failed\n");
    }

    int set_result = canvas_set(result, -1, x, y, width, height, title);
    if (set_result != CANVAS_OK)
    {
        CANVAS_RETURN_ERR(set_result, "window configuration failed\n");
    }

    canvas_time_init(&canvas_info.canvas[result].time);

    _canvas_get_window_display(result);

    canvas_info.canvas[result].cursor = CANVAS_CURSOR_ARROW;

    CANVAS_RETURN(result);
}

int canvas(int64_t x, int64_t y, int64_t width, int64_t height, const char *title)
{
    CANVAS_ENTER_FUNC();
    CANVAS_PARANOID_OP("canvas");

    int result = _canvas_gpu_init();
    if (result != CANVAS_OK)
    {
        CANVAS_RETURN_ERR(result, "GPU initialization failed\n");
    }

    result = canvas_window(x, y, width, height, title);

    if (result < 0)
    {
        CANVAS_RETURN_ERR(result, "canvas window creation failed\n");
    }

    canvas_color(result, (float[]){0.0f, 0.0f, 0.0f, 1.0f});

    int gpu_result = _canvas_gpu_new_window(result);
    if (gpu_result != CANVAS_OK)
    {
        CANVAS_RETURN_ERR(gpu_result, "GPU window setup failed\n");
    }

    CANVAS_PARANOID_CHECK();
    CANVAS_RETURN(result);
}

int canvas_close(int window_id)
{
    CANVAS_ENTER_FUNC();
    CANVAS_PARANOID_OP("canvas_close");
    CANVAS_BOUNDS(window_id);

    if (!canvas_info.canvas[window_id]._valid)
        CANVAS_RETURN(CANVAS_OK);

    canvas_info.canvas[window_id]._valid = false;

    _canvas_close(window_id);

#if CANVAS_VALIDATION >= 5
    // Poison the closed window's memory to catch use-after-close
    uint64_t saved_head = canvas_info.canvas[window_id]._canary_head;
    uint64_t saved_tail = canvas_info.canvas[window_id]._canary_tail;
#endif

    canvas_info.canvas[window_id] = (canvas_type){0};
    _canvas_data[window_id] = (canvas_data){0};

#if CANVAS_VALIDATION >= 5
    // Restore canaries after zeroing
    canvas_info.canvas[window_id]._canary_head = saved_head;
    canvas_info.canvas[window_id]._canary_tail = saved_tail;
#endif

    CANVAS_RETURN(CANVAS_OK);
}

int canvas_set_update_callback(int window_id, canvas_update_callback callback)
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);
#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT(canvas_info.canvas[window_id]._canary_head == CANVAS_CANARY_HEAD);
    CANVAS_ASSERT(canvas_info.canvas[window_id]._canary_tail == CANVAS_CANARY_TAIL);
#endif

    canvas_info.canvas[window_id].update = callback;
    CANVAS_RETURN(CANVAS_OK);
}

int canvas_color(int window_id, const float color[4])
{
    CANVAS_ENTER_FUNC();
    CANVAS_VALID(window_id);
    CANVAS_ASSERT_NOT_NULL(color);

#if CANVAS_VALIDATION >= 5
    // Validate color values are sane floats
    for (int i = 0; i < 4; i++)
    {
        CANVAS_ASSERT_VALID_FLOAT(color[i]);
        if (color[i] < 0.0f || color[i] > 1.0f)
        {
            CANVAS_WARN("color[%d] = %f out of [0,1] range\n", i, color[i]);
        }
    }
#endif

    canvas_info.canvas[window_id].clear[0] = color[0];
    canvas_info.canvas[window_id].clear[1] = color[1];
    canvas_info.canvas[window_id].clear[2] = color[2];
    canvas_info.canvas[window_id].clear[3] = color[3];
    CANVAS_RETURN(CANVAS_OK);
}

//
//
// Time

void canvas_time_update(canvas_time_data *time)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(time);

#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT_NOT_POISONED(time, sizeof(canvas_time_data));
    CANVAS_ASSERT_VALID_DOUBLE(time->current);
    CANVAS_ASSERT_VALID_DOUBLE(time->last);
    CANVAS_ASSERT_VALID_DOUBLE(time->delta);
    CANVAS_ASSERT_RANGE(time->frame_index, 0, 59);
#endif

    time->current = canvas_get_time(time);
    time->raw_delta = time->current - time->last;

    if (time->raw_delta > 0.1)
    {
        time->raw_delta = 0.1;
    }

    const double smoothing = 0.95;
    if (time->frame == 0)
    {
        time->delta = time->raw_delta;
    }
    else
    {
        time->delta = time->delta * smoothing +
                      time->raw_delta * (1.0 - smoothing);
    }

    time->times[time->frame_index] = time->raw_delta;
    time->frame_index = (time->frame_index + 1) % 60;

    double avg_frame_time = 0.0;
    for (int i = 0; i < 60; i++)
    {
        avg_frame_time += time->times[i];
    }
    avg_frame_time /= 60.0;
    time->fps = (avg_frame_time > 0.0) ? (1.0 / avg_frame_time) : 0.0;

    time->last = time->current;
    time->frame++;
    CANVAS_RETURN_VOID();
}

int canvas_time_fixed_step(canvas_time_data *time, double fixed_dt, int max_steps)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(time);

#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT_NOT_POISONED(time, sizeof(canvas_time_data));
    CANVAS_ASSERT_VALID_DOUBLE(fixed_dt);
    CANVAS_ASSERT_VALID_DOUBLE(time->delta);
    CANVAS_ASSERT_VALID_DOUBLE(time->accumulator);
    CANVAS_ASSERT_POSITIVE(fixed_dt);
    CANVAS_ASSERT_POSITIVE(max_steps);
#endif

    time->accumulator += time->delta;

    int steps = 0;
    while (time->accumulator >= fixed_dt && steps < max_steps)
    {
        time->accumulator -= fixed_dt;
        steps++;
    }

    time->alpha = time->accumulator / fixed_dt;

    CANVAS_RETURN(steps);
}

void canvas_limit_fps(canvas_time_data *time, double target_fps)
{
    CANVAS_ENTER_FUNC();
    CANVAS_ASSERT_NOT_NULL(time);

#if CANVAS_VALIDATION >= 5
    CANVAS_ASSERT_VALID_DOUBLE(target_fps);
    if (target_fps > 1000.0)
    {
        CANVAS_WARN("suspicious target_fps: %f\n", target_fps);
    }
#endif

    if (target_fps <= 0.0)
        CANVAS_RETURN_VOID();

    double target_frame_time = 1.0 / target_fps;
    double elapsed = canvas_get_time(time) - (time->current - time->delta);
    double remaining = target_frame_time - elapsed;

    if (remaining > 0.0)
    {
        if (remaining > 0.002)
        {
            canvas_sleep(remaining - 0.002);
        }

        while (canvas_get_time(time) - (time->current - time->delta) < target_frame_time)
        {
            // Busy-wait for accuracy
        }
    }
    CANVAS_RETURN_VOID();
}

#endif // CANVAS_HEADER_ONLY

CANVAS_EXTERN_C_END