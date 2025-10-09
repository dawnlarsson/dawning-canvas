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
        Windows                 \       \       DirectX12   -lgdi32 -luser32 -mwindows -ldwmapi -ldxgi -ld3d12
        MacOS                   \       \       Metal       -framework Cocoa -framework QuartzCore -framework Metal
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

// prevents FMT to mangle the file
CANVAS_EXTERN_C_BEGIN

#ifndef MAX_CANVAS
#define MAX_CANVAS 16
#endif

#ifndef MAX_DISPLAYS
#define MAX_DISPLAYS 8
#endif

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef void *canvas_library_handle;
typedef void *canvas_window_handle;
typedef void (*canvas_update_callback)(int window);
typedef struct
{
    uint64_t start;
    double current;     // Current time in seconds since start
    double delta;       // Delta time in seconds
    double raw_delta;   // Unsmoothed delta time
    double fps;         // Current FPS
    uint64_t frame;     // Frame counter
    double accumulator; // For fixed timestep
    double alpha;       // Interpolation factor for fixed timestep
    double last;        // Last time value
    double times[60];   // For FPS smoothing
    int frame_index;    // Index for frame times
} canvas_time_data;

int canvas(int x, int y, int width, int height, const char *title);
int canvas_window(int x, int y, int width, int height, const char *title);

int canvas_startup();
int canvas_color(int window, const float color[4]);
int canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title);

int canvas_minimize(int window);
int canvas_maximize(int window);
int canvas_restore(int window);
int canvas_close(int window);

int canvas_run(canvas_update_callback update);
int canvas_set_update_callback(int window, canvas_update_callback callback);
void canvas_limit_fps(canvas_time_data *time, double target_fps);
void canvas_sleep(double seconds);

int canvas_quit();

// internal api
void canvas_main_loop();
int _canvas_platform();
int _canvas_update();
int _canvas_window(int x, int y, int width, int height, const char *title);
int _canvas_gpu_init();
int _canvas_gpu_new_window(int window_id);
int _canvas_window_resize(int window_id);
void _canvas_time_init(canvas_time_data *time);
void _canvas_time_update(canvas_time_data *time);
double _canvas_get_time(canvas_time_data *time);
int _canvas_primary_display_index();

typedef struct
{
    int index, x, y, width, height, display;
    bool resize, close, titlebar, os_moved, os_resized;
    bool minimized, maximized;
    float clear[4];
    const char *title;
    canvas_window_handle window;
    canvas_update_callback update;
    canvas_time_data time;
} canvas_type;

typedef struct
{
    bool primary;
    int x, y, width, height;
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

int CANVAS_STATUS = CANVAS_OK;

#ifndef CANVAS_HEADER_ONLY

canvas_update_callback canvas_default_update_callback;

double canvas_limit_mainloop_fps = 240.0;
int _canvas_display_count = 0;
int _canvas_highest_refresh_rate = 60;

bool _canvas_init_platform = false;
bool _canvas_init_gpu = false;
bool _canvas_post_init_ran = false;
bool _canvas_os_timed = false;
bool _canvas_displays_changed = false;
bool _canvas_quit = false;

canvas_type _canvas[MAX_CANVAS];
canvas_display _canvas_displays[MAX_DISPLAYS];
canvas_time_data canvas_main_time = {0};

#if defined(__APPLE__)

#include <TargetConditionals.h>
#include <objc/objc.h>
#include <objc/message.h>
#include <time.h>
#include <mach/mach_time.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>

static mach_timebase_info_data_t _canvas_timebase = {0};

typedef void *objc_id;
typedef void *objc_sel;
static objc_id _mac_pool = NULL;
static objc_id _mac_app = NULL;
static objc_id _mac_device = NULL;
static objc_id _metal_queue = NULL;

typedef struct
{
    objc_id view;
    objc_id layer;
    double scale;
} canvas_data;

canvas_data _canvas_data[MAX_CANVAS];

typedef struct
{
    double r, g, b, a;
} _MTLClearColor;

typedef struct
{
    double x, y, w, h;
} _CGRect;

extern objc_id MTLCreateSystemDefaultDevice(void);

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

static LARGE_INTEGER _canvas_qpc_frequency = {0};
static LARGE_INTEGER _canvas_start_counter = {0};

HINSTANCE _win_instance = NULL;
Atom _win_main_class = NULL;

ID3D12Device *_win_device = NULL;
ID3D12CommandQueue *_win_cmdQueue = NULL;
IDXGIFactory4 *_win_factory = NULL;
ID3D12CommandAllocator *_win_cmdAllocator = NULL;
ID3D12GraphicsCommandList *_win_cmdList = NULL;
ID3D12DescriptorHeap *_win_rtvHeap = NULL;
ID3D12Fence *_win_fence = NULL;
UINT64 _win_fence_value = 0;
HANDLE _win_fence_event = NULL;
UINT _win_rtvDescriptorSize = 0;

typedef struct
{
    IDXGISwapChain3 *swapChain;
    ID3D12Resource *backBuffers[2];
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
} canvas_data;

canvas_data _canvas_data[MAX_CANVAS];

#endif

#if defined(__linux__)

#include <time.h>
#include "dlfcn.h"

canvas_library_handle canvas_lib_wayland;
canvas_library_handle canvas_lib_x11;
canvas_library_handle canvas_lib_xrandr;

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
    struct wl_display *(*wl_display_connect)(const char *);
    void (*wl_display_disconnect)(struct wl_display *);
    int (*wl_display_dispatch)(struct wl_display *);
    int (*wl_display_roundtrip)(struct wl_display *);
    void *(*wl_registry_bind)(struct wl_registry *, uint32_t, const struct wl_interface *, uint32_t);
    int (*wl_registry_add_listener)(struct wl_registry *, const void *, void *);
    void (*wl_registry_destroy)(struct wl_registry *);
    struct wl_surface *(*wl_compositor_create_surface)(struct wl_compositor *);
    void (*wl_surface_destroy)(struct wl_surface *);
    void (*wl_compositor_destroy)(struct wl_compositor *);
    void (*wl_proxy_destroy)(struct wl_proxy *);
    const struct wl_interface *wl_compositor_interface;
} wl;

static struct
{
    Display *(*XOpenDisplay)(const char *);
    int (*XCloseDisplay)(Display *);
    Window (*XCreateSimpleWindow)(Display *, Window, int, int, unsigned int, unsigned int,
                                  unsigned int, unsigned long, unsigned long);
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
} x11;

#define LOAD_X11(name)                                         \
    x11.name = dlsym(canvas_lib_x11, #name);                   \
    if (!x11.name)                                             \
    {                                                          \
        CANVAS_ERR("Failed to load " #name ": %s", dlerror()); \
        dlclose(canvas_lib_x11);                               \
        return CANVAS_ERR_LOAD_SYMBOL;                         \
    }

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

    Atom x11_wm_delete_window;
    Atom x11_wm_protocols;
    Atom x11_wm_state;
    Atom x11_net_wm_state;
    Atom x11_net_wm_state_maximized_horz;
    Atom x11_net_wm_state_maximized_vert;
    Atom x11_net_wm_state_fullscreen;
    bool x11_atoms_initialized;
} canvas_data;

typedef struct
{
    int x, y;
    int width, height;
    int border_width;
    int depth;
    void *visual;
    Window root;
    int c_class;
    int bit_gravity;
    int win_gravity;
    int backing_store;
    unsigned long backing_planes;
    unsigned long backing_pixel;
    int save_under;
    long your_event_mask;
    long all_event_masks;
    long do_not_propagate_mask;
    int override_redirect;
    void *screen;
} XWindowAttributes;

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

Display *_canvas_display = 0;

bool _canvas_x11_flush = false;
bool _canvas_using_wayland = false;

canvas_data _canvas_data[MAX_CANVAS];

#endif

#ifndef CANVAS_NO_LOG
#include <stdio.h>

#define CANVAS_INFO(...) printf("[CANVAS - INF] " __VA_ARGS__)

#ifndef CANVAS_LOG_DEBUG
#define CANVAS_VERBOSE(...) printf("[CANVAS - INF] " __VA_ARGS__ "\n")
#define CANVAS_WARN(...) printf("[CANVAS - WARN] " __VA_ARGS__)
#define CANVAS_ERR(...) printf("[CANVAS - ERR] (%s:%d) " __VA_ARGS__, __FILE__, __LINE__)
#define CANVAS_DBG(...) printf("[CANVAS - DEBUG] (%s:%d) " __VA_ARGS__ "\n", __FILE__, __LINE__)
#else
#define CANVAS_VERBOSE(...)
#define CANVAS_WARN(...)
#define CANVAS_ERR(...)
#define CANVAS_DBG(...)
#endif

#else
#define CANVAS_INFO(...)
#define CANVAS_ERR(...)
#define CANVAS_VERBOSE(...)
#define CANVAS_WARN(...)
#define CANVAS_DBG(...)
#endif

#define CANVAS_BOGUS(window_id)                      \
    if (window_id < 0 || window_id >= MAX_CANVAS)    \
    {                                                \
        CANVAS_ERR("bogus window: %d\n", window_id); \
        return CANVAS_INVALID;                       \
    }

#define CANVAS_DISPLAY_BOGUS(display_id)               \
    if (display_id < 0 || display_id >= MAX_DISPLAYS)  \
    {                                                  \
        CANVAS_ERR("bogus display: %d\n", display_id); \
        return CANVAS_INVALID;                         \
    }

int _canvas_get_free()
{
    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (_canvas[i].window == NULL)
            return i;
    }

    CANVAS_WARN("no free window slots\n");
    return CANVAS_ERR_NO_FREE;
}

int _canvas_window_index(void *window)
{
    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (_canvas[i].window == window)
            return i;
    }

    CANVAS_WARN("bogus get_window_index: %p\n", window);
    return CANVAS_INVALID;
}

//
//
//
#if defined(__APPLE__)

int canvas_minimize(int window_id)
{
    CANVAS_BOGUS(window_id);

    objc_id window = _canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to minimize: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    ((MSG_void_id_id)objc_msgSend)(window, sel_c("miniaturize:"), (objc_id)0);
    _canvas[window_id].minimized = true;
    _canvas[window_id].maximized = false;

    return CANVAS_OK;
}

int canvas_maximize(int window_id)
{
    CANVAS_BOGUS(window_id);

    objc_id window = _canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to maximize: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    bool is_zoomed = ((bool (*)(objc_id, objc_sel))objc_msgSend)(window, sel_c("isZoomed"));

    if (!is_zoomed)
    {
        ((MSG_void_id_id)objc_msgSend)(window, sel_c("zoom:"), (objc_id)0);
        _canvas[window_id].maximized = true;
    }

    _canvas[window_id].minimized = false;

    return CANVAS_OK;
}

int canvas_restore(int window_id)
{
    CANVAS_BOGUS(window_id);

    objc_id window = _canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to restore: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    if (_canvas[window_id].minimized)
    {
        ((MSG_void_id_id)objc_msgSend)(window, sel_c("deminiaturize:"), (objc_id)0);
    }
    else if (_canvas[window_id].maximized)
    {
        ((MSG_void_id_id)objc_msgSend)(window, sel_c("zoom:"), (objc_id)0);
    }

    _canvas[window_id].minimized = false;
    _canvas[window_id].maximized = false;

    return CANVAS_OK;
}

int _canvas_close(int window_id)
{
    CANVAS_BOGUS(window_id);

    objc_id window = _canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to close: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    // cleanup metal layer and view
    if (_canvas_data[window_id].layer)
    {
        ((MSG_void_id)objc_msgSend)(_canvas_data[window_id].layer, sel_c("release"));
        _canvas_data[window_id].layer = NULL;
    }

    if (_canvas_data[window_id].view)
    {
        ((MSG_void_id)objc_msgSend)(_canvas_data[window_id].view, sel_c("release"));
        _canvas_data[window_id].view = NULL;
    }

    ((MSG_void_id)objc_msgSend)(window, sel_c("close"));
    ((MSG_void_id)objc_msgSend)(window, sel_c("release"));

    return CANVAS_OK;
}

int _canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title)
{
    CANVAS_BOGUS(window_id);

    objc_id window = _canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to set: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    CANVAS_DISPLAY_BOGUS(display);
    y = _canvas_displays[display].height - (y + height);

    if (x >= 0 && y >= 0 && width > 0 && height > 0)
    {
        _CGRect rect = {(double)x, (double)y, (double)width, (double)height};
        typedef void (*MSG_void_id_rect_bool)(objc_id, objc_sel, _CGRect, int);
        ((MSG_void_id_rect_bool)objc_msgSend)(window, sel_c("setFrame:display:"), rect, 1);
    }

    if (title)
    {
        objc_id nsTitle = ((MSG_id_id_charp)objc_msgSend)(cls("NSString"), sel_c("stringWithUTF8String:"), title);
        if (nsTitle)
            ((MSG_void_id_id)objc_msgSend)(window, sel_c("setTitle:"), nsTitle);
    }

    return CANVAS_OK;
}

int _canvas_get_window_display(int window_id)
{
    CANVAS_BOGUS(window_id);

    objc_id window = _canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to get display: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    objc_id screen = ((MSG_id_id)objc_msgSend)(window, sel_c("screen"));
    if (!screen)
    {
        CANVAS_ERR("no screen to get display: %d\n", window_id);
        return CANVAS_ERR_GET_DISPLAY;
    }

    objc_id device_desc = ((MSG_id_id)objc_msgSend)(screen, sel_c("deviceDescription"));
    if (!device_desc)
    {
        CANVAS_ERR("no device description to get display: %d\n", window_id);
        return CANVAS_ERR_GET_GPU;
    }

    objc_id screen_number_key = ((MSG_id_id_charp)objc_msgSend)(
        cls("NSString"),
        sel_c("stringWithUTF8String:"),
        "NSScreenNumber");

    objc_id display_id_obj = ((MSG_id_id_id)objc_msgSend)(
        device_desc,
        sel_c("objectForKey:"),
        screen_number_key);

    if (!display_id_obj)
        return CANVAS_OK;

    unsigned long display_id = ((MSG_ulong_id)objc_msgSend)(display_id_obj, sel_c("unsignedIntValue"));

    uint32_t max_displays = MAX_DISPLAYS;
    CGDirectDisplayID displays[MAX_DISPLAYS];
    uint32_t display_count = 0;

    CGGetActiveDisplayList(max_displays, displays, &display_count);

    for (uint32_t i = 0; i < display_count && i < MAX_DISPLAYS; i++)
    {
        if (displays[i] != (CGDirectDisplayID)display_id)
            continue;

        _canvas[window_id].display = i;
        return i;
    }

    return CANVAS_OK;
}

int _canvas_refresh_displays()
{
    _canvas_display_count = 0;
    _canvas_highest_refresh_rate = 60;
    _canvas_displays_changed = false;

    uint32_t max_displays = MAX_DISPLAYS;
    CGDirectDisplayID displays[MAX_DISPLAYS];
    uint32_t display_count = 0;

    CGError err = CGGetActiveDisplayList(max_displays, displays, &display_count);
    if (err != kCGErrorSuccess)
    {
        CANVAS_ERR("get display list: %d\n", err);
        return CANVAS_ERR_GET_DISPLAY;
    }

    CGDirectDisplayID main_display = CGMainDisplayID();

    for (uint32_t i = 0; i < display_count && i < MAX_DISPLAYS; i++)
    {
        CGDirectDisplayID display_id = displays[i];
        CGRect bounds = CGDisplayBounds(display_id);
        CGDisplayModeRef mode = CGDisplayCopyDisplayMode(display_id);
        double refresh_rate = 60.0;

        if (mode)
        {
            refresh_rate = CGDisplayModeGetRefreshRate(mode);

            if (refresh_rate == 0.0)
                refresh_rate = 60.0;

            CGDisplayModeRelease(mode);
        }

        _canvas_displays[i].primary = (display_id == main_display);
        _canvas_displays[i].x = (int)bounds.origin.x;
        _canvas_displays[i].y = (int)bounds.origin.y;
        _canvas_displays[i].width = (int)bounds.size.width;
        _canvas_displays[i].height = (int)bounds.size.height;
        _canvas_displays[i].refresh_rate = (int)refresh_rate;

        if ((int)refresh_rate > _canvas_highest_refresh_rate)
            _canvas_highest_refresh_rate = (int)refresh_rate;

        _canvas_display_count++;
    }

    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (_canvas[i].window)
            _canvas_get_window_display(i);
    }

    return _canvas_display_count;
}

void _canvas_display_reconfigure_callback(CGDirectDisplayID display, CGDisplayChangeSummaryFlags flags, void *userInfo)
{
    if (flags & (kCGDisplayAddFlag | kCGDisplayRemoveFlag | kCGDisplaySetModeFlag | kCGDisplayDesktopShapeChangedFlag))
        _canvas_displays_changed = true;
}

bool _canvas_check_display_changes()
{
    if (_canvas_displays_changed)
    {
        _canvas_refresh_displays();
        return true;
    }
    return false;
}

int _canvas_init_displays()
{
    _canvas_display_count = 0;
    _canvas_highest_refresh_rate = 60;
    CGDisplayRegisterReconfigurationCallback(_canvas_display_reconfigure_callback, NULL);

    return _canvas_refresh_displays();
}

void _post_init()
{
    if (_canvas_post_init_ran)
        return;
    _canvas_post_init_ran = 1;
    objc_id poolClass = cls("NSAutoreleasePool");
    if (!poolClass)
        return;
    MSG_id_id m = (MSG_id_id)objc_msgSend;
    objc_id alloc = m(poolClass, sel_c("alloc"));
    _mac_pool = m(alloc, sel_c("init"));
    mach_timebase_info(&_canvas_timebase);
}

int _canvas_platform()
{
    _post_init();

    MSG_id_id m = (MSG_id_id)objc_msgSend;
    _mac_app = m(cls("NSApplication"), sel_c("sharedApplication"));
    if (!_mac_app)
    {
        CANVAS_ERR("get macOS application\n");
        return CANVAS_ERR_GET_PLATFORM;
    }

    ((MSG_void_id_long)objc_msgSend)(_mac_app, sel_c("setActivationPolicy:"), (long)0);
    ((MSG_void_id_bool)objc_msgSend)(_mac_app, sel_c("activateIgnoringOtherApps:"), 1);

    return CANVAS_OK;
}

int _canvas_window(int x, int y, int width, int height, const char *title)
{
    _post_init();
    canvas_startup();

    unsigned long style =
        (1UL << 0) /*Titled*/ | (1UL << 1) /*Closable*/ |
        (1UL << 2) /*Mini*/ | (1UL << 3) /*Resizable*/;

    MSG_id_id m = (MSG_id_id)objc_msgSend;

    objc_id winClass = cls("NSWindow");

    if (!winClass)
    {
        CANVAS_ERR("get NSWindow class\n");
        return CANVAS_ERR_GET_WINDOW;
    }

    objc_id walloc = m(winClass, sel_c("alloc"));

    typedef objc_id (*MSG_initWin)(objc_id, objc_sel, _CGRect, unsigned long, long, int);
    _CGRect rect = {(double)x, (double)y, (double)width, (double)height};
    objc_id window = ((MSG_initWin)objc_msgSend)(walloc,
                                                 sel_c("initWithContentRect:styleMask:backing:defer:"),
                                                 rect, style, (long)2, 0);

    if (!window)
    {
        CANVAS_ERR("create NSWindow\n");
        return CANVAS_ERR_GET_WINDOW;
    }

    ((MSG_void_id_bool)objc_msgSend)(window, sel_c("setTitlebarAppearsTransparent:"), 1);
    ((MSG_void_id_long)objc_msgSend)(window, sel_c("setTitleVisibility:"), 1L /* NSWindowTitleHidden */);

    unsigned long sm = ((MSG_ulong_id)objc_msgSend)(window, sel_c("styleMask"));
    sm |= (1UL << 15); /* NSWindowStyleMaskFullSizeContentView */
    ((MSG_void_id_ulong)objc_msgSend)(window, sel_c("setStyleMask:"), sm);

    if (title)
    {
        objc_id nsTitle = ((MSG_id_id_charp)objc_msgSend)(cls("NSString"), sel_c("stringWithUTF8String:"), title);
        if (nsTitle)
            ((MSG_void_id_id)objc_msgSend)(window, sel_c("setTitle:"), nsTitle);
    }

    ((MSG_void_id_id)objc_msgSend)(window, sel_c("makeKeyAndOrderFront:"), (objc_id)0);

    int idx = _canvas_get_free();
    if (idx < 0)
        return idx;

    _canvas[idx].window = window;
    _canvas[idx].resize = false;
    _canvas[idx].index = idx;

    return idx;
}

int _canvas_gpu_init()
{
    if (_canvas_init_gpu)
        return CANVAS_OK;
    _canvas_init_gpu = 1;

    _mac_device = MTLCreateSystemDefaultDevice();
    if (!_mac_device)
    {
        CANVAS_ERR("get macOS Metal device\n");
        return CANVAS_ERR_GET_GPU;
    }

    _metal_queue = ((MSG_id_id)objc_msgSend)(_mac_device, sel_c("newCommandQueue"));
    return _metal_queue ? 0 : CANVAS_ERR_GET_GPU;
}

int _canvas_update_drawable_size(int window_id)
{
    CANVAS_BOGUS(window_id);

    if (!_canvas_data[window_id].view || !_canvas_data[window_id].layer)
    {
        CANVAS_ERR("no view or layer to update drawable size: %d\n", window_id);
        return CANVAS_ERR_GET_DISPLAY;
    }

    _CGRect b = ((_CGRect (*)(objc_id, objc_sel))objc_msgSend)(_canvas_data[window_id].view, sel_c("bounds"));
    double w = b.w * _canvas_data[window_id].scale;
    double h = b.h * _canvas_data[window_id].scale;

    struct
    {
        double width;
        double height;
    } sz = {w, h};
    ((void (*)(objc_id, objc_sel, typeof(sz)))objc_msgSend)(_canvas_data[window_id].layer, sel_c("setDrawableSize:"), sz);

    return CANVAS_OK;
}

int _canvas_gpu_new_window(int window_id)
{
    CANVAS_BOGUS(window_id);

    objc_id window = _canvas[window_id].window;

    MSG_id_id m = (MSG_id_id)objc_msgSend;

    objc_id view = m(cls("NSView"), sel_c("alloc"));
    view = ((MSG_id_id_rect)objc_msgSend)(view, sel_c("initWithFrame:"), (_CGRect){0, 0, 800, 600});
    if (!view)
    {
        CANVAS_ERR("create NSView for window: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    ((MSG_void_id_bool)objc_msgSend)(view, sel_c("setWantsLayer:"), 1);

    objc_id layer = m(cls("CAMetalLayer"), sel_c("alloc"));
    layer = m(layer, sel_c("init"));

    if (!layer)
    {
        CANVAS_ERR("create CAMetalLayer for window: %d\n", window_id);
        return CANVAS_ERR_GET_GPU;
    }

    ((MSG_void_id_id)objc_msgSend)(layer, sel_c("setDevice:"), _mac_device);
    ((MSG_void_id_long)objc_msgSend)(layer, sel_c("setPixelFormat:"), 80L /*BGRA8Unorm*/);
    ((MSG_void_id_bool)objc_msgSend)(layer, sel_c("setFramebufferOnly:"), 1);
    ((MSG_void_id_bool)objc_msgSend)(layer, sel_c("setAllowsNextDrawableTimeout:"), 0);

    double scale = 1.0;
    objc_id screen = m(window, sel_c("screen"));
    if (screen)
        scale = ((MSG_dbl_id)objc_msgSend)(screen, sel_c("backingScaleFactor"));
    ((void (*)(objc_id, objc_sel, double))objc_msgSend)(layer, sel_c("setContentsScale:"), scale);

    ((MSG_void_id_id)objc_msgSend)(view, sel_c("setLayer:"), layer);
    ((MSG_void_id_id)objc_msgSend)(window, sel_c("setContentView:"), view);

    _canvas_data[window_id].view = view;
    _canvas_data[window_id].layer = layer;
    _canvas_data[window_id].scale = scale;

    return window_id;
}

void _canvas_gpu_draw_all()
{
    if (!_metal_queue)
    {
        CANVAS_VERBOSE("no metal command queue to draw\n");
        return;
    }

    for (int i = 0; i < MAX_CANVAS; ++i)
    {
        if (!_canvas[i].window || !_canvas_data[i].layer)
            continue;

        _canvas_update_drawable_size(i);

        objc_id layer = _canvas_data[i].layer;
        if (!layer)
            continue;

        objc_id drawable = ((MSG_id_id)objc_msgSend)(layer, sel_c("nextDrawable"));

        if (!drawable)
            continue;

        objc_id texture = ((MSG_id_id)objc_msgSend)(drawable, sel_c("texture"));

        objc_id rpd = ((MSG_id_id)objc_msgSend)(cls("MTLRenderPassDescriptor"), sel_c("renderPassDescriptor"));
        objc_id caps = ((MSG_id_id)objc_msgSend)(rpd, sel_c("colorAttachments"));
        objc_id att0 = ((MSG_id_id_ulong)objc_msgSend)(caps, sel_c("objectAtIndexedSubscript:"), 0UL);

        ((MSG_void_id_id)objc_msgSend)(att0, sel_c("setTexture:"), texture);
        ((MSG_void_id_ulong)objc_msgSend)(att0, sel_c("setLoadAction:"), 2UL);  /* Clear */
        ((MSG_void_id_ulong)objc_msgSend)(att0, sel_c("setStoreAction:"), 1UL); /* Store */

        _MTLClearColor cc = {_canvas[i].clear[0], _canvas[i].clear[1], _canvas[i].clear[2], _canvas[i].clear[3]};
        ((MSG_void_id_clear)objc_msgSend)(att0, sel_c("setClearColor:"), cc);

        objc_id cmd = ((MSG_id_id)objc_msgSend)(_metal_queue, sel_c("commandBuffer"));
        objc_id enc = ((MSG_id_id_id)objc_msgSend)(cmd, sel_c("renderCommandEncoderWithDescriptor:"), rpd);
        ((MSG_void_id)objc_msgSend)(enc, sel_c("endEncoding"));

        ((MSG_void_id_id)objc_msgSend)(cmd, sel_c("presentDrawable:"), drawable);
        ((MSG_void_id)objc_msgSend)(cmd, sel_c("commit"));
    }
}

int _canvas_update()
{
    _post_init();

    _canvas_check_display_changes();

    objc_id poolClass = cls("NSAutoreleasePool");
    objc_id framePool = NULL;
    if (poolClass)
    {
        objc_id tmp = ((MSG_id_id)objc_msgSend)(poolClass, sel_c("alloc"));
        framePool = ((MSG_id_id)objc_msgSend)(tmp, sel_c("init"));
    }

    objc_id ns_mode = ((MSG_id_id_charp)objc_msgSend)(cls("NSString"), sel_c("stringWithUTF8String:"), "kCFRunLoopDefaultMode");
    objc_id distantPast = ((MSG_id_id)objc_msgSend)(cls("NSDate"), sel_c("distantPast"));

    typedef objc_id (*MSG_nextEvent)(objc_id, objc_sel, unsigned long long, objc_id, objc_id, signed char);
    MSG_nextEvent nextEvent = (MSG_nextEvent)objc_msgSend;

    for (;;)
    {
        objc_id ev = nextEvent(_mac_app, sel_c("nextEventMatchingMask:untilDate:inMode:dequeue:"),
                               ~0ULL, distantPast, ns_mode, (signed char)1);
        if (!ev)
            break;

        unsigned long eventType = ((MSG_ulong_id)objc_msgSend)(ev, sel_c("type"));

        objc_id eventWindow = ((MSG_id_id)objc_msgSend)(ev, sel_c("window"));

        if (eventWindow)
        {
            int window_idx = _canvas_window_index(eventWindow);

            if (window_idx >= 0)
            {
                // NSEventType constants
                // NSEventTypeLeftMouseDragged = 6
                // NSEventTypeOtherMouseDragged = 27
                // NSEventTypeRightMouseDragged = 7

                switch (eventType)
                {
                case 6:  // NSEventTypeLeftMouseDragged
                case 27: // NSEventTypeOtherMouseDragged
                case 7:  // NSEventTypeRightMouseDragged
                {
                    _CGRect frame = ((MSG_rect_id)objc_msgSend)(eventWindow, sel_c("frame"));

                    if ((int)frame.x != _canvas[window_idx].x ||
                        (int)frame.y != _canvas[window_idx].y)
                    {
                        _canvas[window_idx].os_moved = true;
                        _canvas[window_idx].x = (int)frame.x;
                        _canvas[window_idx].y = (int)frame.y;
                    }

                    if ((int)frame.w != _canvas[window_idx].width ||
                        (int)frame.h != _canvas[window_idx].height)
                    {
                        _canvas[window_idx].resize = true;
                        _canvas[window_idx].os_resized = true;
                        _canvas[window_idx].width = (int)frame.w;
                        _canvas[window_idx].height = (int)frame.h;
                    }
                    break;
                }
                }
            }
        }

        ((MSG_void_id_id)objc_msgSend)(_mac_app, sel_c("sendEvent:"), ev);
    }

    objc_id tracking_mode = ((MSG_id_id_charp)objc_msgSend)(cls("NSString"), sel_c("stringWithUTF8String:"), "NSEventTrackingRunLoopMode");
    for (;;)
    {
        objc_id ev = nextEvent(_mac_app, sel_c("nextEventMatchingMask:untilDate:inMode:dequeue:"),
                               ~0ULL, distantPast, tracking_mode, (signed char)1);
        if (!ev)
            break;
        ((MSG_void_id_id)objc_msgSend)(_mac_app, sel_c("sendEvent:"), ev);
    }

    ((MSG_void_id)objc_msgSend)(_mac_app, sel_c("updateWindows"));

    _canvas_gpu_draw_all();

    if (framePool)
        ((MSG_void_id)objc_msgSend)(framePool, sel_c("drain"));

    return CANVAS_OK;
}

int _canvas_post_update()
{
    return CANVAS_OK;
}

int _canvas_exit()
{
    return CANVAS_OK;
}

double _canvas_get_time(canvas_time_data *time)
{
    uint64_t elapsed = mach_absolute_time() - time->start;
    return (double)elapsed * (double)_canvas_timebase.numer /
           (double)_canvas_timebase.denom / 1e9;
}

void canvas_sleep(double seconds)
{
    struct timespec ts;
    ts.tv_sec = (time_t)seconds;
    ts.tv_nsec = (long)((seconds - (double)ts.tv_sec) * 1e9);
    nanosleep(&ts, NULL);
}

void _canvas_time_init(canvas_time_data *time)
{
    time->start = mach_absolute_time();
    time->current = _canvas_get_time(time);
    time->last = time->current;
    time->frame = 0;

    for (int i = 0; i < 60; i++)
        time->times[i] = 0.0;
}

#endif /* __APPLE__ */

//
//
//
#if defined(_WIN32) || defined(_WIN64)

int canvas_minimize(int window_id)
{
    CANVAS_BOGUS(window_id);

    HWND window = (HWND)_canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to minimize: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    ShowWindow(window, SW_MINIMIZE);
    _canvas[window_id].minimized = true;
    _canvas[window_id].maximized = false;

    return CANVAS_OK;
}

int canvas_maximize(int window_id)
{
    CANVAS_BOGUS(window_id);

    HWND window = (HWND)_canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to maximize: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    ShowWindow(window, SW_MAXIMIZE);
    _canvas[window_id].maximized = true;
    _canvas[window_id].minimized = false;

    return CANVAS_OK;
}

int canvas_restore(int window_id)
{
    CANVAS_BOGUS(window_id);

    HWND window = (HWND)_canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to restore: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    ShowWindow(window, SW_RESTORE);
    _canvas[window_id].minimized = false;
    _canvas[window_id].maximized = false;

    return CANVAS_OK;
}

int _canvas_close(int window_id)
{
    CANVAS_BOGUS(window_id);

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

    DestroyWindow((HWND)_canvas[window_id].window);
    return CANVAS_OK;
}

int _canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title)
{
    CANVAS_BOGUS(window_id);

    HWND window = (HWND)_canvas[window_id].window;

    if (!window)
    {
        CANVAS_ERR("no window to set: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    if (display < 0 || display >= _canvas_display_count)
        display = 0;

    CANVAS_DISPLAY_BOGUS(display);

    int screen_x = _canvas_displays[display].x + x;
    int screen_y = _canvas_displays[display].y + y;

    SetWindowPos(window, NULL, screen_x, screen_y, width, height, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE);

    if (title)
        SetWindowTextA(window, title);

    _canvas[window_id].display = display;

    return CANVAS_OK;
}

int _canvas_get_window_display(int window_id)
{
    CANVAS_BOGUS(window_id);

    HWND window = (HWND)_canvas[window_id].window;

    if (!window)
    {
        CANVAS_ERR("no window to set: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY);

    for (int i = 0; i < _canvas_display_count; i++)
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

        _canvas[window_id].display = i;
        return i;
    }

    return CANVAS_OK;
}

int _canvas_refresh_displays()
{
    _canvas_display_count = 0;
    _canvas_highest_refresh_rate = 60;
    _canvas_displays_changed = false;

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

        _canvas_displays[i].primary = (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) != 0;
        _canvas_displays[i].x = dm.dmPosition.x;
        _canvas_displays[i].y = dm.dmPosition.y;
        _canvas_displays[i].width = dm.dmPelsWidth;
        _canvas_displays[i].height = dm.dmPelsHeight;
        _canvas_displays[i].refresh_rate = dm.dmDisplayFrequency;

        if (dm.dmDisplayFrequency > _canvas_highest_refresh_rate)
            _canvas_highest_refresh_rate = dm.dmDisplayFrequency;

        _canvas_display_count++;
    }

    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (_canvas[i].window)
            _canvas_get_window_display(i);
    }

    return _canvas_display_count;
}

int _canvas_init_displays()
{
    _canvas_display_count = 0;
    _canvas_highest_refresh_rate = 60;

    return _canvas_refresh_displays();
}

double _canvas_get_time(canvas_time_data *time)
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (double)((uint64_t)counter.QuadPart - time->start) /
           (double)_canvas_qpc_frequency.QuadPart;
}

void canvas_sleep(double seconds)
{
    HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (timer)
    {
        LARGE_INTEGER li;
        li.QuadPart = -(LONGLONG)(seconds * 10000000.0); // 100ns units
        SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
    }
}

void _canvas_time_init(canvas_time_data *time)
{
    QueryPerformanceFrequency(&_canvas_qpc_frequency);

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    time->start = (uint64_t)counter.QuadPart;
    time->current = _canvas_get_time(time);
    time->last = time->current;
    time->frame = 0;
    time->frame_index = 0;

    for (int i = 0; i < 60; i++)
        time->times[i] = 0.0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int window_index = _canvas_window_index(hwnd);

    if (window_index < 0)
        return DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_CREATE:
    {
        if (!_canvas[window_index].titlebar)
        {
            DWM_WINDOW_CORNER_PREFERENCE corner = DWMWCP_ROUND;
            DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));

            enum DWM_SYSTEMBACKDROP_TYPE backdrop = DWMSBT_MAINWINDOW;
            DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));

            BOOL useRoundedCorners = TRUE;
            DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));
        }
        return CANVAS_OK;
    }

    case WM_NCCALCSIZE:
    {
        if (wParam == true && !_canvas[window_index].titlebar)
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
        if (!_canvas[window_index].titlebar)
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
    }

    case WM_DISPLAYCHANGE:
    {
        _canvas[window_index].os_moved = true;
        _canvas_displays_changed = true;
        return CANVAS_OK;
    }

    case WM_MOVE:
    {
        // _canvas_update_window_display(window_index);

        return CANVAS_OK;
    }

    case WM_MOVING:
    {
        _canvas[window_index].os_moved = true;

        return CANVAS_OK;
    }

    case WM_SIZE:
    {

        if (wParam != SIZE_MINIMIZED)
        {
            _canvas[window_index].resize = true;

            if (!_canvas_os_timed)
            {
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        return CANVAS_OK;
    }

    case WM_SIZING:
    {

        _canvas[window_index].os_resized = true;

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
                _canvas[window_index].resize = true;
                _canvas_window_resize(window_index);
                _canvas_update();
            }
                */

        return TRUE;
    }

    case WM_ENTERSIZEMOVE:
    {
        if (_canvas_os_timed)
            break;

        _canvas_os_timed = true;
        SetTimer(hwnd, 1, 0, NULL); // 60 fps hard cap due to Windows...
        return CANVAS_OK;
    }

    case WM_EXITSIZEMOVE:
    {
        if (!_canvas_os_timed)
            break;

        _canvas_os_timed = false;
        KillTimer(hwnd, 1);
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
            _canvas[window_index].minimized = true;
        else if (wParam == SC_MAXIMIZE)
            _canvas[window_index].maximized = true;
        else if (wParam == SC_RESTORE)
        {
            _canvas[window_index].minimized = false;
            _canvas[window_index].maximized = false;
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int _canvas_window(int x, int y, int width, int height, const char *title)
{
    canvas_startup();

    int window_id = _canvas_get_free();
    if (window_id < 0)
        return window_id;

    if (!_win_main_class)
    {
        WNDCLASSA wc = {0};
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = _win_instance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszClassName = "CanvasWindowClass";

        _win_main_class = RegisterClassA(&wc);

        if (!_win_main_class)
        {
            CANVAS_ERR("register windows class failed");
            return CANVAS_ERR_GET_PLATFORM;
        }
    }

    DWORD style = WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_VISIBLE;

    _canvas[window_id].index = window_id;

    HWND window = CreateWindowA(
        "CanvasWindowClass",
        title,
        style,
        x, y, width, height,
        NULL, NULL, _win_instance, NULL);

    if (!window)
    {
        CANVAS_ERR("create win32 window")
        return CANVAS_ERR_GET_WINDOW;
    }

    _canvas[window_id].window = window;
    _canvas[window_id].resize = false;
    _canvas[window_id].titlebar = false;

    return window_id;
}

int _canvas_platform()
{
    _win_instance = GetModuleHandle(NULL);

    return CANVAS_OK;
}

int _canvas_update()
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (_win_device)
    {
        for (int i = 0; i < MAX_CANVAS; ++i)
        {
            if (_canvas[i].resize)
            {
                _canvas_window_resize(i);
            }
        }

        _win_cmdAllocator->lpVtbl->Reset(_win_cmdAllocator);
        _win_cmdList->lpVtbl->Reset(_win_cmdList, _win_cmdAllocator, NULL);

        for (int i = 0; i < MAX_CANVAS; ++i)
        {
            if (_canvas[i].window == NULL ||
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
            _win_cmdList->lpVtbl->ResourceBarrier(_win_cmdList, 1, &barrier);

            _win_cmdList->lpVtbl->ClearRenderTargetView(_win_cmdList,
                                                        _canvas_data[i].rtvHandles[backBufferIndex],
                                                        _canvas[i].clear, 0, NULL);

            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            _win_cmdList->lpVtbl->ResourceBarrier(_win_cmdList, 1, &barrier);
        }

        _win_cmdList->lpVtbl->Close(_win_cmdList);
        ID3D12CommandList *cmdLists[] = {(ID3D12CommandList *)_win_cmdList};
        _win_cmdQueue->lpVtbl->ExecuteCommandLists(_win_cmdQueue, 1, cmdLists);

        for (int i = 0; i < MAX_CANVAS; ++i)
        {
            if (_canvas_data[i].swapChain != NULL)
            {
                _canvas_data[i].swapChain->lpVtbl->Present(_canvas_data[i].swapChain, 1, 0);
            }
        }

        _win_fence_value++;
        _win_cmdQueue->lpVtbl->Signal(_win_cmdQueue, _win_fence, _win_fence_value);
        if (_win_fence->lpVtbl->GetCompletedValue(_win_fence) < _win_fence_value)
        {
            _win_fence->lpVtbl->SetEventOnCompletion(_win_fence, _win_fence_value, _win_fence_event);
            WaitForSingleObject(_win_fence_event, INFINITE);
        }
    }

    return CANVAS_OK;
}

int _canvas_gpu_init()
{
    if (_canvas_init_gpu)
        return CANVAS_OK;

    _canvas_init_gpu = 1;

    HRESULT result;

    result = CreateDXGIFactory1(
        &IID_IDXGIFactory4,
        (void **)&_win_factory);

    if (FAILED(result))
    {
        CANVAS_ERR("create dx12 factory");
        return CANVAS_ERR_GET_GPU;
    }

    result = D3D12CreateDevice(
        NULL,
        D3D_FEATURE_LEVEL_11_0,
        &IID_ID3D12Device,
        (void **)&_win_device);

    if (FAILED(result))
    {
        CANVAS_ERR("create dx12 device");
        return CANVAS_ERR_GET_GPU;
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {0};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    result = _win_device->lpVtbl->CreateCommandQueue(
        _win_device,
        &queueDesc,
        &IID_ID3D12CommandQueue,
        (void **)&_win_cmdQueue);

    if (FAILED(result))
    {
        CANVAS_ERR("create command queue");
        return CANVAS_ERR_GET_GPU;
    }

    result = _win_device->lpVtbl->CreateCommandAllocator(
        _win_device,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        &IID_ID3D12CommandAllocator,
        (void **)&_win_cmdAllocator);

    if (FAILED(result))
    {
        CANVAS_ERR("create command alloc");
        return CANVAS_ERR_GET_GPU;
    }

    result = _win_device->lpVtbl->CreateCommandList(
        _win_device,
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        _win_cmdAllocator,
        NULL,
        &IID_ID3D12GraphicsCommandList,
        (void **)&_win_cmdList);

    if (FAILED(result))
    {
        CANVAS_ERR("create command list");
        return CANVAS_ERR_GET_GPU;
    }

    _win_cmdList->lpVtbl->Close(_win_cmdList);

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {0};
    heapDesc.NumDescriptors = MAX_CANVAS * 2;

    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    result = _win_device->lpVtbl->CreateDescriptorHeap(
        _win_device,
        &heapDesc,
        &IID_ID3D12DescriptorHeap,
        (void **)&_win_rtvHeap);

    if (FAILED(result))
    {
        CANVAS_ERR("create descriptor heap")
        return CANVAS_ERR_GET_GPU;
    }
    _win_rtvDescriptorSize = _win_device->lpVtbl->GetDescriptorHandleIncrementSize(
        _win_device,
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    result = _win_device->lpVtbl->CreateFence(
        _win_device,
        0,
        D3D12_FENCE_FLAG_NONE,
        &IID_ID3D12Fence,
        (void **)&_win_fence);

    if (FAILED(result))
    {
        CANVAS_ERR("create fence")
        return CANVAS_ERR_GET_GPU;
    }

    _win_fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (!_win_fence_event)
    {
        CANVAS_ERR("create fence event")
        return CANVAS_ERR_GET_GPU;
    }

    return CANVAS_OK;
}
int _canvas_gpu_new_window(int window_id)
{
    CANVAS_BOGUS(window_id);

    _canvas_data[window_id] = (canvas_data){0};

    HWND window = (HWND)_canvas[window_id].window;

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
    HRESULT result = _win_factory->lpVtbl->CreateSwapChainForHwnd(
        _win_factory,
        (IUnknown *)_win_cmdQueue,
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
        CANVAS_ERR("query swapchain interface\n");
        return CANVAS_ERR_GET_GPU;
    }

    _win_factory->lpVtbl->MakeWindowAssociation(
        _win_factory,
        window,
        DXGI_MWA_NO_ALT_ENTER);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    _win_rtvHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(_win_rtvHeap, &rtvHandle);

    rtvHandle.ptr += window_id * 2 * _win_rtvDescriptorSize;

    for (int i = 0; i < 2; i++)
    {
        result = _canvas_data[window_id].swapChain->lpVtbl->GetBuffer(
            _canvas_data[window_id].swapChain,
            i,
            &IID_ID3D12Resource,
            (void **)&_canvas_data[window_id].backBuffers[i]);

        if (FAILED(result))
        {
            CANVAS_ERR("get swapchain buffer %d\n", i);
            return CANVAS_ERR_GET_GPU;
        }

        _canvas_data[window_id].rtvHandles[i] = rtvHandle;
        _win_device->lpVtbl->CreateRenderTargetView(
            _win_device,
            _canvas_data[window_id].backBuffers[i],
            NULL,
            rtvHandle);
        rtvHandle.ptr += _win_rtvDescriptorSize;
    }

    return CANVAS_OK;
}

int _canvas_window_resize(int window_id)
{
    CANVAS_BOGUS(window_id);

    canvas_data *window = &_canvas_data[window_id];

    if (!window->swapChain || !_canvas[window_id].resize)
    {
        _canvas[window_id].resize = false;
        return CANVAS_OK;
    }

    _canvas[window_id].resize = false;

    _win_fence_value++;
    _win_cmdQueue->lpVtbl->Signal(_win_cmdQueue, _win_fence, _win_fence_value);
    if (_win_fence->lpVtbl->GetCompletedValue(_win_fence) < _win_fence_value)
    {
        _win_fence->lpVtbl->SetEventOnCompletion(_win_fence, _win_fence_value, _win_fence_event);
        WaitForSingleObject(_win_fence_event, INFINITE);
    }

    RECT rect;
    GetClientRect((HWND)_canvas[window_id].window, &rect);
    UINT width = rect.right - rect.left;
    UINT height = rect.bottom - rect.top;

    if (width == 0 || height == 0)
        return CANVAS_OK;

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
        CANVAS_ERR("resize swapchain buffers\n");
        return CANVAS_ERR_GET_GPU;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    _win_rtvHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(_win_rtvHeap, &rtvHandle);
    rtvHandle.ptr += window_id * 2 * _win_rtvDescriptorSize;

    for (int i = 0; i < 2; i++)
    {
        hr = window->swapChain->lpVtbl->GetBuffer(
            window->swapChain, i, &IID_ID3D12Resource,
            (void **)&window->backBuffers[i]);

        if (FAILED(hr))
        {
            CANVAS_ERR("get resized buffer %d\n", i);
            return CANVAS_ERR_GET_GPU;
        }

        window->rtvHandles[i] = rtvHandle;
        _win_device->lpVtbl->CreateRenderTargetView(
            _win_device, window->backBuffers[i], NULL, rtvHandle);
        rtvHandle.ptr += _win_rtvDescriptorSize;
    }

    return CANVAS_OK;
}

int _canvas_post_update()
{
    return CANVAS_OK;
}

int _canvas_exit()
{
    CloseHandle(_win_fence_event);
    return CANVAS_OK;
}

#endif

//
//
//
#if defined(__linux__)

int canvas_minimize(int window_id)
{
    CANVAS_BOGUS(window_id);

    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!_canvas_display)
        {
            CANVAS_ERR("no display connection\n");
            return CANVAS_ERR_GET_DISPLAY;
        }

        Window window = (Window)_canvas[window_id].window;
        if (!window)
        {
            CANVAS_ERR("no window to minimize: %d\n", window_id);
            return CANVAS_ERR_GET_WINDOW;
        }

        int screen = x11.XDefaultScreen(_canvas_display);
        x11.XIconifyWindow(_canvas_display, window, screen);
        x11.XFlush(_canvas_display);
    }

    _canvas[window_id].minimized = true;
    _canvas[window_id].maximized = false;

    return CANVAS_OK;
}

int canvas_maximize(int window_id)
{
    CANVAS_BOGUS(window_id);

    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!_canvas_display)
        {
            CANVAS_ERR("no display connection\n");
            return CANVAS_ERR_GET_DISPLAY;
        }

        Window window = (Window)_canvas[window_id].window;
        if (!window)
        {
            CANVAS_ERR("no window to maximize: %d\n", window_id);
            return CANVAS_ERR_GET_WINDOW;
        }

        int screen = x11.XDefaultScreen(_canvas_display);

        Atom wm_state = x11.XInternAtom(_canvas_display, "_NET_WM_STATE", 0);
        Atom max_horz = x11.XInternAtom(_canvas_display, "_NET_WM_STATE_MAXIMIZED_HORZ", 0);
        Atom max_vert = x11.XInternAtom(_canvas_display, "_NET_WM_STATE_MAXIMIZED_VERT", 0);

        XClientMessageEvent ev = {0};
        ev.type = 33;
        ev.window = window;
        ev.message_type = wm_state;
        ev.format = 32;
        ev.data.l[0] = 1;
        ev.data.l[1] = max_horz;
        ev.data.l[2] = max_vert;
        ev.data.l[3] = 1;

        x11.XSendEvent(_canvas_display, x11.XRootWindow(_canvas_display, screen),
                       0, (1L << 20) | (1L << 19), (XEvent *)&ev);
        x11.XFlush(_canvas_display);
    }

    _canvas[window_id].maximized = true;
    _canvas[window_id].minimized = false;

    return CANVAS_OK;
}

int canvas_restore(int window_id)
{
    CANVAS_BOGUS(window_id);

    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!_canvas_display)
        {
            CANVAS_ERR("no display connection\n");
            return CANVAS_ERR_GET_DISPLAY;
        }

        Window window = (Window)_canvas[window_id].window;
        if (!window)
        {
            CANVAS_ERR("no window to restore: %d\n", window_id);
            return CANVAS_ERR_GET_WINDOW;
        }

        int screen = x11.XDefaultScreen(_canvas_display);

        if (_canvas[window_id].minimized)
        {
            x11.XMapWindow(_canvas_display, window);
        }
        else if (_canvas[window_id].maximized)
        {
            Atom wm_state = x11.XInternAtom(_canvas_display, "_NET_WM_STATE", 0);
            Atom max_horz = x11.XInternAtom(_canvas_display, "_NET_WM_STATE_MAXIMIZED_HORZ", 0);
            Atom max_vert = x11.XInternAtom(_canvas_display, "_NET_WM_STATE_MAXIMIZED_VERT", 0);

            XClientMessageEvent ev = {0};
            ev.type = 33;
            ev.window = window;
            ev.message_type = wm_state;
            ev.format = 32;
            ev.data.l[0] = 0;
            ev.data.l[1] = max_horz;
            ev.data.l[2] = max_vert;
            ev.data.l[3] = 1;

            x11.XSendEvent(_canvas_display, x11.XRootWindow(_canvas_display, screen),
                           0, (1L << 20) | (1L << 19), (XEvent *)&ev);
        }

        x11.XFlush(_canvas_display);
    }

    _canvas[window_id].minimized = false;
    _canvas[window_id].maximized = false;

    return CANVAS_OK;
}

int _canvas_close(int window_id)
{
    CANVAS_BOGUS(window_id);

    if (_canvas_using_wayland)
    {
    }
    else
    {
        Window window = (Window)_canvas[window_id].window;

        if (!window)
        {
            CANVAS_ERR("no window to close: %d\n", window_id);
            return CANVAS_ERR_GET_WINDOW;
        }

        x11.XDestroyWindow(_canvas_display, window);
        x11.XFlush(_canvas_display);
    }

    return CANVAS_OK;
}

int _canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title)
{
    if (!_canvas_display)
    {
        CANVAS_ERR("no display connection\n");
        return CANVAS_ERR_GET_DISPLAY;
    }

    CANVAS_BOGUS(window_id);
    CANVAS_DISPLAY_BOGUS(display);

    if (_canvas_using_wayland)
    {
    }
    else
    {
        Window window = (Window)_canvas[window_id].window;

        if (!window)
        {
            CANVAS_ERR("no window to set: %d\n", window_id);
            return CANVAS_ERR_GET_WINDOW;
        }

        if (title)
        {
            x11.XStoreName(_canvas_display, window, title);
            Atom net_wm_name = x11.XInternAtom(_canvas_display, "_NET_WM_NAME", 0);
            Atom utf8_string = x11.XInternAtom(_canvas_display, "UTF8_STRING", 0);
            x11.XChangeProperty(_canvas_display, window, net_wm_name, utf8_string, 8, 0, (unsigned char *)title, strlen(title));
        }

        x11.XMoveResizeWindow(_canvas_display, window, x, y, width, height);
    }

    return CANVAS_OK;
}

int _canvas_init_displays()
{
    _canvas_display_count = 0;
    _canvas_highest_refresh_rate = 60;

    if (!_canvas_display)
    {
        CANVAS_ERR("no display connection for init\n");
        return CANVAS_ERR_GET_DISPLAY;
    }

    if (_canvas_using_wayland)
    {
    }
    else
    {
        // TODO: Use Xinerama or XRandR for proper multi-monitor support
        int screen = x11.XDefaultScreen(_canvas_display);

        _canvas_displays[0].primary = true;
        _canvas_displays[0].x = 0;
        _canvas_displays[0].y = 0;
        _canvas_displays[0].width = x11.XDisplayWidth(_canvas_display, screen);
        _canvas_displays[0].height = x11.XDisplayHeight(_canvas_display, screen);
        _canvas_displays[0].refresh_rate = 60;

        _canvas_display_count = 1;
    }

    return _canvas_display_count;
}

int _canvas_get_window_display(int window_id)
{
    CANVAS_BOGUS(window_id);

    if (_canvas_using_wayland)
    {
    }
    else
    {
    }

    return CANVAS_OK;
}

int _canvas_window(int x, int y, int width, int height, const char *title)
{
    canvas_startup();

    canvas_window_handle window = 0;

    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!_canvas_display)
        {
            CANVAS_ERR("no display connection for window creation\n");
            return CANVAS_ERR_GET_DISPLAY;
        }

        window = (canvas_window_handle)x11.XCreateSimpleWindow(
            _canvas_display,
            x11.XDefaultRootWindow(_canvas_display),
            x, y, width, height, 0,
            x11.XBlackPixel(_canvas_display, 0),
            x11.XWhitePixel(_canvas_display, 0));

        if (!window)
        {
            CANVAS_ERR("create x11 window\n");
            return CANVAS_ERR_GET_WINDOW;
        }

        x11.XMapWindow(_canvas_display, (Window)window);

        long event_mask = (1L << 0) |  // KeyPressMask
                          (1L << 1) |  // KeyReleaseMask
                          (1L << 2) |  // ButtonPressMask
                          (1L << 3) |  // ButtonReleaseMask
                          (1L << 5) |  // PointerMotionMask
                          (1L << 15) | // ExposureMask
                          (1L << 17) | // StructureNotifyMask
                          (1L << 19);  // FocusChangeMask

        x11.XSelectInput(_canvas_display, (Window)window, event_mask);
    }

    int window_id = _canvas_get_free();
    if (window_id < 0)
        return window_id;

    _canvas[window_id].window = (canvas_window_handle)window;
    _canvas[window_id].resize = false;
    _canvas[window_id].index = window_id;

    if (!_canvas_using_wayland)
    {
        _canvas_data[window_id].x11_wm_protocols = x11.XInternAtom(_canvas_display, "WM_PROTOCOLS", 0);
        _canvas_data[window_id].x11_wm_delete_window = x11.XInternAtom(_canvas_display, "WM_DELETE_WINDOW", 0);
        _canvas_data[window_id].x11_wm_state = x11.XInternAtom(_canvas_display, "WM_STATE", 0);
        _canvas_data[window_id].x11_net_wm_state = x11.XInternAtom(_canvas_display, "_NET_WM_STATE", 0);
        _canvas_data[window_id].x11_net_wm_state_maximized_horz = x11.XInternAtom(_canvas_display, "_NET_WM_STATE_MAXIMIZED_HORZ", 0);
        _canvas_data[window_id].x11_net_wm_state_maximized_vert = x11.XInternAtom(_canvas_display, "_NET_WM_STATE_MAXIMIZED_VERT", 0);
        _canvas_data[window_id].x11_net_wm_state_fullscreen = x11.XInternAtom(_canvas_display, "_NET_WM_STATE_FULLSCREEN", 0);
        x11.XSetWMProtocols(_canvas_display, (Window)_canvas[window_id].window, &_canvas_data[window_id].x11_wm_delete_window, 1);

        _canvas_data[window_id].x11_atoms_initialized = true;
    }

    return window_id;
}

int _canvas_init_wayland()
{
    canvas_lib_wayland = dlopen("libwayland-client.so.0", RTLD_LAZY | RTLD_NOW | RTLD_GLOBAL);

    if (!canvas_lib_wayland)
    {
        canvas_lib_wayland = dlopen("libwayland-client.so", RTLD_LAZY | RTLD_NOW | RTLD_GLOBAL);
    }

    if (!canvas_lib_wayland)
    {
        dlclose(canvas_lib_wayland);
        CANVAS_ERR("libwayland-client.so.0 or libwayland-client.so not found");
        return CANVAS_ERR_LOAD_LIBRARY;
    }

    _canvas_using_wayland = true;
}

int _canvas_init_x11()
{
    canvas_lib_x11 = dlopen("libX11.so.6", RTLD_LAZY);

    if (!canvas_lib_x11)
    {
        canvas_lib_x11 = dlopen("libX11.so", RTLD_LAZY);
    }

    if (!canvas_lib_x11)
    {
        dlclose(canvas_lib_x11);
        CANVAS_ERR("libX11.so.6 or libX11.so not found");
        return CANVAS_ERR_LOAD_LIBRARY;
    }

    LOAD_X11(XOpenDisplay);
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

    _canvas_display = x11.XOpenDisplay(NULL);

    if (!_canvas_display)
    {
        CANVAS_ERR("open x11 display\n");
        return CANVAS_ERR_GET_DISPLAY;
    }

    // Try to load XRandR for proper multi-monitor support
    canvas_lib_xrandr = dlopen("libXrandr.so.2", RTLD_LAZY);

    if (!canvas_lib_xrandr)
        canvas_lib_xrandr = dlopen("libXrandr.so", RTLD_LAZY);

    if (!canvas_lib_xrandr)
    {
        dlclose(canvas_lib_xrandr);
        CANVAS_WARN("libXrandr.so.2 or libXrandr.so not found");
        return CANVAS_OK;
    }

    typedef void *(*XRRGetScreenResourcesCurrentFunc)(Display *, Window);
    typedef void (*XRRFreeScreenResourcesFunc)(void *);
    typedef void *(*XRRGetCrtcInfoFunc)(Display *, void *, unsigned long);
    typedef void (*XRRFreeCrtcInfoFunc)(void *);

    XRRGetScreenResourcesCurrentFunc XRRGetScreenResourcesCurrent = dlsym(canvas_lib_xrandr, "XRRGetScreenResourcesCurrent");
    XRRFreeScreenResourcesFunc XRRFreeScreenResources = dlsym(canvas_lib_xrandr, "XRRFreeScreenResources");
    XRRGetCrtcInfoFunc XRRGetCrtcInfo = dlsym(canvas_lib_xrandr, "XRRGetCrtcInfo");
    XRRFreeCrtcInfoFunc XRRFreeCrtcInfo = dlsym(canvas_lib_xrandr, "XRRFreeCrtcInfo");

    if (!XRRGetScreenResourcesCurrent || !XRRFreeScreenResources || !XRRGetCrtcInfo || !XRRFreeCrtcInfo)
    {
        dlclose(canvas_lib_xrandr);
        CANVAS_WARN("failed to load Xrandr symbols");
        return CANVAS_OK;
    }

    Window root = x11.XDefaultRootWindow(_canvas_display);

    typedef struct
    {
        void *timestamp;
        void *configTimestamp;
        int ncrtc;
        unsigned long *crtcs;
        // ...
    } XRRScreenResources;

    typedef struct
    {
        long timestamp;
        int x, y;
        unsigned int width, height;
        // ...
    } XRRCrtcInfo;

    XRRScreenResources *sr = XRRGetScreenResourcesCurrent(_canvas_display, root);

    if (!sr)
    {
        dlclose(canvas_lib_xrandr);
        CANVAS_WARN("failed call XRRGetScreenResourcesCurrent");
        return CANVAS_OK;
    }

    for (int i = 0; i < sr->ncrtc && i < MAX_DISPLAYS; i++)
    {
        XRRCrtcInfo *ci = XRRGetCrtcInfo(_canvas_display, sr, sr->crtcs[i]);

        if (ci && ci->width > 0 && ci->height > 0)
        {
            _canvas_displays[_canvas_display_count].x = ci->x;
            _canvas_displays[_canvas_display_count].y = ci->y;
            _canvas_displays[_canvas_display_count].width = ci->width;
            _canvas_displays[_canvas_display_count].height = ci->height;
            _canvas_displays[_canvas_display_count].refresh_rate = 60;
            _canvas_displays[_canvas_display_count].primary = (i == 0);

            _canvas_display_count++;
        }

        if (ci)
            XRRFreeCrtcInfo(ci);
    }

    XRRFreeScreenResources(sr);

    return CANVAS_OK;
}

int _canvas_platform()
{
    //     int load_result = _canvas_init_wayland();

    return _canvas_init_x11();
}

int _canvas_update()
{
    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!_canvas_display)
        {
            CANVAS_VERBOSE("no display connection for update\n");
            return CANVAS_ERR_GET_DISPLAY;
        }

        XEvent event;

        while (x11.XPending(_canvas_display))
        {
            x11.XNextEvent(_canvas_display, &event);

            int window_idx = -1;
            if (event.type >= 2 && event.type <= 35)
            {
                _x11_event *base_event = (_x11_event *)&event;
                window_idx = _canvas_window_index((void *)base_event->window);
            }

            if (window_idx < 0)
                continue;

            switch (event.type)
            {
            case X11_ConfigureNotify:
            {
                XWindowAttributes attrs;
                if (x11.XGetWindowAttributes(_canvas_display,
                                             (Window)_canvas[window_idx].window,
                                             &attrs))
                {
                    if (attrs.width != _canvas[window_idx].width ||
                        attrs.height != _canvas[window_idx].height)
                    {
                        _canvas[window_idx].resize = true;
                        _canvas[window_idx].width = attrs.width;
                        _canvas[window_idx].height = attrs.height;
                    }

                    if (attrs.x != _canvas[window_idx].x ||
                        attrs.y != _canvas[window_idx].y)
                    {
                        _canvas[window_idx].x = attrs.x;
                        _canvas[window_idx].y = attrs.y;
                    }
                }
                break;
            }

            case X11_ClientMessage:
            {
                XClientMessageEvent *cm = (XClientMessageEvent *)&event;

                // Check for window close request
                if ((Atom)cm->data.l[0] == _canvas_data[window_idx].x11_wm_delete_window)
                {
                    _canvas[window_idx].close = true;
                }
                break;
            }

            case X11_UnmapNotify:
            {
                // Window was minimized/hidden
                _canvas[window_idx].minimized = true;
                _canvas[window_idx].maximized = false;
                break;
            }

            case X11_MapNotify:
            {
                // Window was restored/shown
                _canvas[window_idx].minimized = false;
                break;
            }

            case X11_Expose:
            {
                // Window needs redraw (we handle this automatically)
                break;
            }
            }
        }
    }

    return CANVAS_OK;
}

int _canvas_gpu_init()
{
    if (_canvas_init_gpu)
        return CANVAS_OK;
    _canvas_init_gpu = 1;

    // Select & init backend OpenGL / Vulkan

    return CANVAS_OK;
}

int _canvas_gpu_new_window(int window_id)
{
    CANVAS_BOGUS(window_id);

    return CANVAS_OK;
}

int _canvas_post_update()
{
    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!_canvas_display)
        {
            CANVAS_ERR("no display connection for post update\n");
            return CANVAS_ERR_GET_DISPLAY;
        }

        x11.XFlush(_canvas_display);
    }

    return CANVAS_OK;
}

int _canvas_exit()
{
    x11.XCloseDisplay(_canvas_display);

    dlclose(canvas_lib_x11);
    dlclose(canvas_lib_xrandr);

    return CANVAS_OK;
}

double _canvas_get_time(canvas_time_data *time)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t now = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    return (double)(now - time->start) / 1e9;
}

void canvas_sleep(double seconds)
{
    struct timespec ts;
    ts.tv_sec = (time_t)seconds;
    ts.tv_nsec = (long)((seconds - (double)ts.tv_sec) * 1e9);
    clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
}

void _canvas_time_init(canvas_time_data *time)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    time->start = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    time->current = _canvas_get_time(time);
    time->last = time->current;
    time->frame = 0;

    for (int i = 0; i < 60; i++)
        time->times[i] = 0.0;
}

#endif // _linux_

int _canvas_primary_display_index(void)
{
    for (int i = 0; i < _canvas_display_count; ++i)
        if (_canvas_displays[i].primary)
            return i;
    return 0;
}

int canvas_startup()
{
    if (_canvas_init_platform)
        return CANVAS_OK;

    _canvas_init_platform = true;

    _canvas_time_init(&canvas_main_time);

    for (int i = 0; i < MAX_DISPLAYS; ++i)
    {
        _canvas_displays[i] = (canvas_display){0};
    }

    for (int i = 0; i < MAX_CANVAS; ++i)
    {
        _canvas[i] = (canvas_type){0};
        _canvas_data[i] = (canvas_data){0};
    }

    int result = _canvas_platform();
    if (result != CANVAS_OK)
    {
        CANVAS_STATUS = CANVAS_FAIL;
        CANVAS_ERR("platform initialization failed\n");
        return result;
    }

    result = _canvas_init_displays();
    if (result < 0)
    {
        CANVAS_ERR("display initialization failed\n");
        return result;
    }

    return CANVAS_OK;
}

void canvas_main_loop()
{
    _canvas_time_update(&canvas_main_time);

    _canvas_update();

    for (int i = 0; i < MAX_CANVAS; ++i)
    {
        if (_canvas[i].window == 0)
            continue;

        if (_canvas[i].update)
        {
            _canvas[i].update(i);
        }
        else if (canvas_default_update_callback)
        {
            canvas_default_update_callback(i);
        }
    }

    _canvas_post_update();

    canvas_limit_fps(&canvas_main_time, canvas_limit_mainloop_fps);
}

int canvas_exit()
{
    return _canvas_exit();
}

int canvas_quit()
{
    CANVAS_INFO("quitting canvas");
    _canvas_quit = 1;
    return CANVAS_OK;
}

int canvas_run(canvas_update_callback default_callback)
{
    if (CANVAS_STATUS != CANVAS_OK)
    {
        CANVAS_ERR("refusing run, initialization failed.");
        return CANVAS_STATUS;
    }

    canvas_default_update_callback = default_callback;

    while (_canvas_quit == 0)
    {
        _canvas_os_timed = false;
        canvas_main_loop();
    }

    return CANVAS_OK;
}

// display:     -1 = primary display
// x:           -1 = centered
// y:           -1 = centered
// width:       window width in pixels
// height:      window height in pixels
// title:       window title string
int canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title)
{
    CANVAS_BOGUS(window_id);

    if (_canvas_display_count <= 0)
    {
        CANVAS_ERR("no displays available\n");
        return CANVAS_ERR_GET_DISPLAY;
    }

    if (display < 0 || display >= _canvas_display_count)
    {
        display = _canvas_primary_display_index();
    }

    _canvas[window_id].display = display;
    _canvas[window_id].width = width;
    _canvas[window_id].height = height;
    _canvas[window_id].title = title;

    _canvas[window_id].os_moved = false;
    _canvas[window_id].os_resized = false;

    _canvas[window_id].x = x;
    _canvas[window_id].y = y;

    int target_x = x;
    int target_y = y;

    CANVAS_DISPLAY_BOGUS(display);

    if (x == -1)
        target_x = _canvas_displays[display].width / 2 - width / 2;

    if (y == -1)
        target_y = _canvas_displays[display].height / 2 - height / 2;

    return _canvas_set(window_id, display, target_x, target_y, width, height, title);
}

int canvas_window(int x, int y, int width, int height, const char *title)
{
    int result = _canvas_window(x, y, width, height, title);

    if (result < 0)
    {
        CANVAS_ERR("window creation failed\n");
        return result;
    }

    int set_result = canvas_set(result, -1, x, y, width, height, title);
    if (set_result != CANVAS_OK)
    {
        CANVAS_ERR("window configuration failed\n");
        return set_result;
    }

    _canvas_time_init(&_canvas[result].time);

    _canvas_get_window_display(result);

    return result;
}

int canvas(int x, int y, int width, int height, const char *title)
{
    int result = _canvas_gpu_init();
    if (result != CANVAS_OK)
    {
        CANVAS_ERR("GPU initialization failed\n");
        return result;
    }

    result = canvas_window(x, y, width, height, title);

    if (result < 0)
    {
        CANVAS_ERR("canvas window creation failed\n");
        return result;
    }

    canvas_color(result, (float[]){0.0f, 0.0f, 0.0f, 1.0f});

    int gpu_result = _canvas_gpu_new_window(result);
    if (gpu_result != CANVAS_OK)
    {
        CANVAS_ERR("GPU window setup failed\n");
        return gpu_result;
    }

    return result;
}

int canvas_close(int window_id)
{
    CANVAS_BOGUS(window_id);

    _canvas_close(window_id);

    _canvas[window_id].window = NULL;
    _canvas[window_id] = (canvas_type){0};
    _canvas_data[window_id] = (canvas_data){0};

    return CANVAS_OK;
}

int canvas_set_update_callback(int window_id, canvas_update_callback callback)
{
    CANVAS_BOGUS(window_id);

    _canvas[window_id].update = callback;
    return CANVAS_OK;
}

int canvas_color(int window_id, const float color[4])
{
    CANVAS_BOGUS(window_id);
    _canvas[window_id].clear[0] = color[0];
    _canvas[window_id].clear[1] = color[1];
    _canvas[window_id].clear[2] = color[2];
    _canvas[window_id].clear[3] = color[3];
    return CANVAS_OK;
}

//
//
// Time

void _canvas_time_update(canvas_time_data *time)
{
    time->current = _canvas_get_time(time);
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
}

int _canvas_time_fixed_step(canvas_time_data *time, double fixed_dt, int max_steps)
{
    time->accumulator += time->delta;

    int steps = 0;
    while (time->accumulator >= fixed_dt && steps < max_steps)
    {
        time->accumulator -= fixed_dt;
        steps++;
    }

    time->alpha = time->accumulator / fixed_dt;

    return steps;
}

void canvas_limit_fps(canvas_time_data *time, double target_fps)
{
    if (target_fps <= 0.0)
        return;

    double target_frame_time = 1.0 / target_fps;
    double elapsed = _canvas_get_time(time) - (time->current - time->delta);
    double remaining = target_frame_time - elapsed;

    if (remaining > 0.0)
    {
        if (remaining > 0.002)
        {
            canvas_sleep(remaining - 0.002);
        }

        while (_canvas_get_time(time) - (time->current - time->delta) < target_frame_time)
        {
            // Busy-wait for accuracy
        }
    }
}

#endif // CANVAS_HEADER_ONLY

CANVAS_EXTERN_C_END