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

#ifndef MAX_CANVAS_TITLE
#define MAX_CANVAS_TITLE 256
#endif

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

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

typedef enum
{
    ARROW,
    TEXT,
    CROSSHAIR,
    HAND,
    SIZE_NS,
    SIZE_EW,
    SIZE_NESW,
    SIZE_NWSE,
    SIZE_ALL,
    NOT_ALLOWED,
    WAIT,
} canvas_cursor_type;

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

int canvas_exit();

void canvas_time_init(canvas_time_data *time);
void canvas_time_update(canvas_time_data *time);
double canvas_get_time(canvas_time_data *time);
int canvas_time_fixed_step(canvas_time_data *time, double fixed_dt, int max_steps);
int canvas_cursor(int window_id, canvas_cursor_type cursor);

// internal api
void canvas_main_loop();
int _canvas_platform();
int _canvas_update();
int _canvas_window(int x, int y, int width, int height, const char *title);
int _canvas_gpu_init();
int _canvas_gpu_new_window(int window_id);
int _canvas_window_resize(int window_id);
int _canvas_primary_display_index();

typedef struct
{
    int index, x, y, width, height, display;
    bool resize, close, titlebar, os_moved, os_resized;
    bool minimized, maximized, vsync, _valid;
    float clear[4];
    char title[MAX_CANVAS_TITLE];
    canvas_window_handle window;
    canvas_update_callback update;
    canvas_time_data time;
    canvas_cursor_type cursor, active_cursor;
} canvas_type;

typedef struct
{
    bool primary;
    int x, y, width, height, scale;
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

#ifndef CANVAS_HEADER_ONLY

typedef struct
{
    bool init, init_gpu, init_post, os_timed, auto_exit, quit, display_changed;
    int display_count, limit_fps, highest_refresh_rate;

    canvas_type canvas[MAX_CANVAS];
    canvas_display display[MAX_DISPLAYS];

    canvas_update_callback update_callback;
    canvas_time_data time;

} canvas_context_type;

canvas_context_type canvas_info = (canvas_context_type){0};

#if defined(__APPLE__)

#include <TargetConditionals.h>
#include <objc/objc.h>
#include <objc/message.h>
#include <time.h>
#include <mach/mach_time.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>
#include <dlfcn.h>

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
} canvas_data;

typedef struct
{
    objc_id pool;
    objc_id app;
    mach_timebase_info_data_t timebase;

    // metal
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
    return ((msg_send_id_id)objc_msgSend)(obj, sel_c(sel));
}

static inline void msg_void(objc_id obj, const char *sel)
{
    ((msg_send_void_id)objc_msgSend)(obj, sel_c(sel));
}

static inline void msg_void_id(objc_id obj, const char *sel, objc_id arg)
{
    ((msg_send_void_id_id)objc_msgSend)(obj, sel_c(sel), arg);
}

static inline void msg_void_bool(objc_id obj, const char *sel, bool val)
{
    ((msg_send_void_id_bool)objc_msgSend)(obj, sel_c(sel), val ? 1 : 0);
}

static inline void msg_void_long(objc_id obj, const char *sel, long val)
{
    ((msg_send_void_id_long)objc_msgSend)(obj, sel_c(sel), val);
}

static inline double msg_dbl(objc_id obj, const char *sel)
{
    return ((msg_send_dbl_id)objc_msgSend)(obj, sel_c(sel));
}

static inline unsigned long msg_ulong(objc_id obj, const char *sel)
{
    return ((msg_send_ulong_id)objc_msgSend)(obj, sel_c(sel));
}

static inline bool msg_bool(objc_id obj, const char *sel)
{
    typedef bool (*msg_send_bool)(objc_id, objc_sel);
    return ((msg_send_bool)objc_msgSend)(obj, sel_c(sel));
}

static inline void msg_void_rect_bool(objc_id obj, const char *sel, _CGRect rect, bool display)
{
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
    typedef objc_id (*msg_send_id_id)(objc_id, objc_sel, objc_id);
    return ((msg_send_id_id)objc_msgSend)(obj, sel_c(sel), arg);
}

static inline _CGRect msg_rect(objc_id obj, const char *sel)
{
    typedef _CGRect (*msg_send_rect)(objc_id, objc_sel);
    return ((msg_send_rect)objc_msgSend)(obj, sel_c(sel));
}

static inline void msg_void_size(objc_id obj, const char *sel, double width, double height)
{
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
    typedef objc_id (*msg_send_rect)(objc_id, objc_sel, _CGRect);
    return ((msg_send_rect)objc_msgSend)(obj, sel_c(sel), rect);
}

static inline void msg_void_double(objc_id obj, const char *sel, double val)
{
    typedef void (*msg_send_dbl)(objc_id, objc_sel, double);
    ((msg_send_dbl)objc_msgSend)(obj, sel_c(sel), val);
}

static inline objc_id msg_id_ulong(objc_id obj, const char *sel, unsigned long val)
{
    typedef objc_id (*msg_send_ulong)(objc_id, objc_sel, unsigned long);
    return ((msg_send_ulong)objc_msgSend)(obj, sel_c(sel), val);
}

static inline void msg_void_clear_color(objc_id obj, const char *sel, _MTLClearColor color)
{
    typedef void (*msg_send_clear)(objc_id, objc_sel, _MTLClearColor);
    ((msg_send_clear)objc_msgSend)(obj, sel_c(sel), color);
}

static inline objc_id msg_id_id_descriptor(objc_id obj, const char *sel, objc_id descriptor)
{
    typedef objc_id (*msg_send_desc)(objc_id, objc_sel, objc_id);
    return ((msg_send_desc)objc_msgSend)(obj, sel_c(sel), descriptor);
}

static inline objc_id canvas_next_event(objc_id app, unsigned long long mask, objc_id until_date, objc_id mode, bool dequeue)
{
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
    [ARROW] = "arrowCursor",
    [TEXT] = "IBeamCursor",
    [CROSSHAIR] = "crosshairCursor",
    [HAND] = "pointingHandCursor",
    [SIZE_NS] = "resizeUpDownCursor",
    [SIZE_EW] = "resizeLeftRightCursor",
    [SIZE_NESW] = "closedHandCursor",
    [SIZE_NWSE] = "closedHandCursor",
    [SIZE_ALL] = "closedHandCursor",
    [NOT_ALLOWED] = "operationNotAllowedCursor",
    [WAIT] = "arrowCursor",
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
} _canvas_platform_windows;

typedef struct
{
    IDXGISwapChain3 *swapChain;
    ID3D12Resource *backBuffers[2];
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
} canvas_data;

canvas_data _canvas_data[MAX_CANVAS];
_canvas_platform_windows canvas_win32;

#endif

#if defined(__linux__)

#define CANVAS_VULKAN

#include <time.h>
#include <dlfcn.h>

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

static struct
{
    canvas_library_handle library;

    bool cursors_loaded;
    unsigned long cursors[11];

    Display *(*XOpenDisplay)(const char *);
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

#ifndef CANVAS_NO_LOG
#include <stdio.h>

#define CANVAS_INFO(...) printf("[CANVAS - INF] " __VA_ARGS__)

#ifndef CANVAS_LOG_DEBUG
#define CANVAS_VERBOSE(...) printf("[CANVAS - INF] " __VA_ARGS__)
#define CANVAS_WARN(...) printf("[CANVAS - WARN] " __VA_ARGS__)
#define CANVAS_ERR(...) printf("[CANVAS - ERR] " __VA_ARGS__)
#define CANVAS_DBG(...) printf("[CANVAS - DBG] " __VA_ARGS__)
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

#define C_RTLD_NOW 0x00002
#define C_RTLD_LOCAL 0x00001

#define CANVAS_BOUNDS(window_id)                     \
    if (window_id < 0 || window_id >= MAX_CANVAS)    \
    {                                                \
        CANVAS_ERR("bogus window: %d\n", window_id); \
        return CANVAS_INVALID;                       \
    }

#define CANVAS_DISPLAY_BOUNDS(display_id)              \
    if (display_id < 0 || display_id >= MAX_DISPLAYS)  \
    {                                                  \
        CANVAS_ERR("bogus display: %d\n", display_id); \
        return CANVAS_INVALID;                         \
    }

#define CANVAS_VALID(window_id)                            \
    CANVAS_BOUNDS(window_id)                               \
    if (!canvas_info.canvas[window_id]._valid)             \
    {                                                      \
        CANVAS_ERR("window %d is not valid\n", window_id); \
        return CANVAS_INVALID;                             \
    }

int _canvas_get_free()
{
    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (!canvas_info.canvas[i]._valid)
            return i;
    }

    CANVAS_WARN("no free window slots\n");
    return CANVAS_ERR_NO_FREE;
}

int _canvas_window_index(void *window)
{
    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (canvas_info.canvas[i].window == window)
            return i;
    }

    CANVAS_WARN("bogus get_window_index: %p\n", window);
    return CANVAS_INVALID;
}

void *canvas_library_load(const char **names, int count)
{
    for (int i = 0; i < count; ++i)
    {
#if defined(_WIN32)
        HMODULE h = LoadLibraryA(names[i]);
#else
        void *h = dlopen(names[i], RTLD_NOW | RTLD_LOCAL);
#endif
        if (h)
            return h;
    }
    return NULL;
}

void *canvas_library_symbol(void *lib, const char *sym)
{
#if defined(_WIN32)
    return (void *)GetProcAddress((HMODULE)lib, sym);
#else
    return dlsym(lib, sym);
#endif
}

void canvas_library_close(void *lib)
{
#if defined(_WIN32)
    FreeLibrary((HMODULE)lib);
#else
    dlclose(lib);
#endif
}

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

#define VK_CHECK(result, msg)                             \
    if ((result) != VK_SUCCESS)                           \
    {                                                     \
        CANVAS_ERR("%s (VkResult: %d)\n", msg, (result)); \
        return CANVAS_FAIL;                               \
    }

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

    VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT];
    VkFence images_in_flight[MAX_SWAPCHAIN_IMAGES];
    uint32_t current_frame;

    VkRenderPass render_pass;
    bool needs_resize;
    bool initialized;
} canvas_vulkan_window;

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

    // Device functions
    PFN_vkCreateDevice vkCreateDevice;
    PFN_vkDestroyDevice vkDestroyDevice;
    PFN_vkGetDeviceQueue vkGetDeviceQueue;
    PFN_vkDeviceWaitIdle vkDeviceWaitIdle;

    // Surface functions
    PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;

    // Platform-specific surface creation
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

    // Swapchain functions
    PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
    PFN_vkQueuePresentKHR vkQueuePresentKHR;

    // Command buffer functions
    PFN_vkCreateCommandPool vkCreateCommandPool;
    PFN_vkDestroyCommandPool vkDestroyCommandPool;
    PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
    PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
    PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
    PFN_vkEndCommandBuffer vkEndCommandBuffer;
    PFN_vkResetCommandBuffer vkResetCommandBuffer;

    // Synchronization
    PFN_vkCreateSemaphore vkCreateSemaphore;
    PFN_vkDestroySemaphore vkDestroySemaphore;
    PFN_vkCreateFence vkCreateFence;
    PFN_vkDestroyFence vkDestroyFence;
    PFN_vkWaitForFences vkWaitForFences;
    PFN_vkResetFences vkResetFences;

    // Command submission
    PFN_vkQueueSubmit vkQueueSubmit;
    PFN_vkQueueWaitIdle vkQueueWaitIdle;

    // Render pass and framebuffer
    PFN_vkCreateRenderPass vkCreateRenderPass;
    PFN_vkDestroyRenderPass vkDestroyRenderPass;
    PFN_vkCreateFramebuffer vkCreateFramebuffer;
    PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
    PFN_vkCreateImageView vkCreateImageView;
    PFN_vkDestroyImageView vkDestroyImageView;

    // Command buffer recording
    PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
    PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
    PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;

    // Debug utils (optional, for validation)
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

static canvas_vulkan_window vk_windows[MAX_CANVAS] = {0};
const char *vulkan_library_names[canvas_vulkan_names] = canvas_vulkan_library_names;

static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data)
{
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        CANVAS_WARN("vulkan validation: %s\n", callback_data->pMessage);

    return VK_FALSE;
}

static int vk_setup_debug_messenger()
{
    if (!vk_info.validation_enabled || !vk_info.vkCreateDebugUtilsMessengerEXT)
        return CANVAS_OK;

    VkDebugUtilsMessengerCreateInfoEXT create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = vk_debug_callback;

    VkResult result = vk_info.vkCreateDebugUtilsMessengerEXT(vk_info.instance, &create_info, NULL, &vk_info.debug_messenger);
    VK_CHECK(result, "failed to setup debug messenger");

    return CANVAS_OK;
}

static bool vk_check_validation_layers()
{
    if (!vk_info.vkEnumerateInstanceLayerProperties)
    {
        CANVAS_WARN("vkEnumerateInstanceLayerProperties not available\n");
        return false;
    }

    uint32_t layer_count = 0;
    VkResult result = vk_info.vkEnumerateInstanceLayerProperties(&layer_count, NULL);

    if (result != VK_SUCCESS)
    {
        CANVAS_WARN("failed to enumerate validation layers (code: %d), disabling validation\n", result);
        return false;
    }

    if (layer_count == 0)
    {
        CANVAS_VERBOSE("no validation layers available\n");
        return false;
    }

    VkLayerProperties *available_layers = malloc(layer_count * sizeof(VkLayerProperties));
    if (!available_layers)
    {
        CANVAS_WARN("failed to allocate memory for layer properties\n");
        return false;
    }

    result = vk_info.vkEnumerateInstanceLayerProperties(&layer_count, available_layers);
    if (result != VK_SUCCESS)
    {
        CANVAS_WARN("failed to get layer properties (code: %d), disabling validation\n", result);
        free(available_layers);
        return false;
    }

    bool found = false;
    const char *validation_layer = "VK_LAYER_KHRONOS_validation";

    for (uint32_t i = 0; i < layer_count; i++)
    {
        if (strcmp(validation_layer, available_layers[i].layerName) == 0)
        {
            found = true;
            break;
        }
    }

    free(available_layers);

    if (!found)
        CANVAS_VERBOSE("VK_LAYER_KHRONOS_validation not found, validation disabled\n");

    return found;
}

static int vk_load_instance_functions()
{
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

    return CANVAS_OK;
}

static int vk_load_device_functions()
{
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

    return CANVAS_OK;
}

static bool vk_check_device_extension_support(VkPhysicalDevice device)
{
    uint32_t extension_count = 0;
    vk_info.vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);

    if (extension_count == 0)
        return false;

    VkExtensionProperties *available_extensions = malloc(extension_count * sizeof(VkExtensionProperties));
    if (!available_extensions)
        return false;

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
    return has_swapchain;
}

static bool vk_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface, int *graphics_family, int *present_family)
{
    uint32_t queue_family_count = 0;
    vk_info.vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    if (queue_family_count == 0)
        return false;

    VkQueueFamilyProperties *queue_families = malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
    if (!queue_families)
        return false;

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
    return (*graphics_family >= 0 && *present_family >= 0);
}

static bool vk_is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR test_surface)
{
    int graphics_family, present_family;

    if (!vk_find_queue_families(device, test_surface, &graphics_family, &present_family))
        return false;

    if (!vk_check_device_extension_support(device))
        return false;

    uint32_t format_count = 0;
    vk_info.vkGetPhysicalDeviceSurfaceFormatsKHR(device, test_surface, &format_count, NULL);

    uint32_t present_mode_count = 0;
    vk_info.vkGetPhysicalDeviceSurfacePresentModesKHR(device, test_surface, &present_mode_count, NULL);

    return (format_count > 0 && present_mode_count > 0);
}

static VkResult vk_select_physical_device(VkSurfaceKHR test_surface)
{
    uint32_t device_count = 0;
    VkResult result = vk_info.vkEnumeratePhysicalDevices(vk_info.instance, &device_count, NULL);
    if (result != VK_SUCCESS || device_count == 0)
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkPhysicalDevice *devices = malloc(device_count * sizeof(VkPhysicalDevice));
    if (!devices)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    result = vk_info.vkEnumeratePhysicalDevices(vk_info.instance, &device_count, devices);
    if (result != VK_SUCCESS)
    {
        free(devices);
        return result;
    }

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
            return VK_SUCCESS;
        }
    }

    free(devices);
    return VK_ERROR_INITIALIZATION_FAILED;
}

static int vk_create_logical_device()
{
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

    vk_info.vkGetDeviceQueue(vk_info.device, vk_info.graphics_family, 0, &vk_info.graphics_queue);
    vk_info.vkGetDeviceQueue(vk_info.device, vk_info.present_family, 0, &vk_info.present_queue);

    return CANVAS_OK;
}

int canvas_backend_vulkan_init()
{
    if (vk_info.instance)
        return CANVAS_OK;

    CANVAS_INFO("initializing Vulkan backend\n");

    vk_info.library = canvas_library_load(vulkan_library_names, canvas_vulkan_names);

    if (!vk_info.library)
    {
        CANVAS_ERR("failed to load Vulkan library\n");
        return CANVAS_ERR_LOAD_LIBRARY;
    }

    vk_info.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)canvas_library_symbol(vk_info.library, "vkGetInstanceProcAddr");
    vk_info.vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)canvas_library_symbol(vk_info.library, "vkGetDeviceProcAddr");

    if (!vk_info.vkGetInstanceProcAddr)
    {
        CANVAS_ERR("failed to load vkGetInstanceProcAddr\n");
        canvas_library_close(vk_info.library);
        return CANVAS_ERR_LOAD_SYMBOL;
    }

    vk_info.vkCreateInstance = (PFN_vkCreateInstance)vk_info.vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    vk_info.vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)vk_info.vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceLayerProperties");
    vk_info.vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)vk_info.vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceExtensionProperties");

    if (!vk_info.vkCreateInstance || !vk_info.vkEnumerateInstanceLayerProperties || !vk_info.vkEnumerateInstanceExtensionProperties)
    {
        CANVAS_ERR("failed to load Vulkan functions\n");
        canvas_library_close(vk_info.library);
        return CANVAS_ERR_LOAD_SYMBOL;
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
        CANVAS_ERR("vkCreateInstance failed with code: %d\n", result);
        return result;
    }

    result = vk_load_instance_functions();
    if (result != CANVAS_OK)
    {
        CANVAS_ERR("failed to load vulkan instance functions\n");
        vk_info.vkDestroyInstance(vk_info.instance, NULL);
        canvas_library_close(vk_info.library);
        return result;
    }

    vk_setup_debug_messenger();

    CANVAS_INFO("vulkan instance created successfully\n");

    return CANVAS_OK;
}

static int vk_create_surface(int window_id, VkSurfaceKHR *surface)
{
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
        // TODO: wayland display and surface
        // create_info.display = wl.display;
        // create_info.surface = (struct wl_surface*)canvas_info.canvas[window_id].window;

        result = vk_info.vkCreateWaylandSurfaceKHR(vk_info.instance, &create_info, NULL, surface);
        VK_CHECK(result, "failed to create Wayland surface");
    }
    else if (vk_info.vkCreateXlibSurfaceKHR)
    {
        VkXlibSurfaceCreateInfoKHR create_info = {0};
        create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        create_info.dpy = x11.display;
        create_info.window = (Window)canvas_info.canvas[window_id].window;

        result = vk_info.vkCreateXlibSurfaceKHR(vk_info.instance, &create_info, NULL, surface);
        VK_CHECK(result, "failed to create Xlib surface");
    }
    else
    {
        CANVAS_ERR("no surface creation function available for Linux\n");
        return CANVAS_FAIL;
    }

#elif defined(__APPLE__)
    if (!_canvas_data[window_id].layer)
    {
        CANVAS_ERR("no Metal layer available for surface creation\n");
        return CANVAS_FAIL;
    }

    VkMetalSurfaceCreateInfoEXT create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    create_info.pLayer = _canvas_data[window_id].layer;

    result = vk_info.vkCreateMetalSurfaceEXT(vk_info.instance, &create_info, NULL, surface);
    VK_CHECK(result, "failed to create Metal surface");

#else
    CANVAS_ERR("unsupported platform for Vulkan surface creation\n");
    return CANVAS_FAIL;
#endif

    return CANVAS_OK;
}

static SwapchainSupportDetails vk_query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapchainSupportDetails details = {0};

    vk_info.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    vk_info.vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.format_count, NULL);
    if (details.format_count > 0)
    {
        details.formats = malloc(details.format_count * sizeof(VkSurfaceFormatKHR));
        vk_info.vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.format_count, details.formats);
    }

    vk_info.vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.present_mode_count, NULL);
    if (details.present_mode_count > 0)
    {
        details.present_modes = malloc(details.present_mode_count * sizeof(VkPresentModeKHR));
        vk_info.vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.present_mode_count, details.present_modes);
    }

    return details;
}

static void vk_cleanup_swapchain_support_details(SwapchainSupportDetails *details)
{
    if (details->formats)
        free(details->formats);
    if (details->present_modes)
        free(details->present_modes);
}

static VkSurfaceFormatKHR vk_choose_surface_format(const VkSurfaceFormatKHR *formats, uint32_t format_count)
{
    for (uint32_t i = 0; i < format_count; i++)
    {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return formats[i];
    }

    return formats[0];
}

static VkPresentModeKHR vk_choose_present_mode(const VkPresentModeKHR *present_modes, uint32_t mode_count, bool vsync)
{
    if (vsync)
        return VK_PRESENT_MODE_FIFO_KHR;

    for (uint32_t i = 0; i < mode_count; i++)
    {
        if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            return VK_PRESENT_MODE_MAILBOX_KHR;
    }

    for (uint32_t i = 0; i < mode_count; i++)
    {
        if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D vk_choose_swap_extent(const VkSurfaceCapabilitiesKHR *capabilities, int window_id)
{
    if (capabilities->currentExtent.width != UINT32_MAX)
        return capabilities->currentExtent;

    VkExtent2D actual_extent = {.width = (uint32_t)canvas_info.canvas[window_id].width, .height = (uint32_t)canvas_info.canvas[window_id].height};

    if (actual_extent.width < capabilities->minImageExtent.width)
        actual_extent.width = capabilities->minImageExtent.width;

    else if (actual_extent.width > capabilities->maxImageExtent.width)
        actual_extent.width = capabilities->maxImageExtent.width;

    if (actual_extent.height < capabilities->minImageExtent.height)
        actual_extent.height = capabilities->minImageExtent.height;

    else if (actual_extent.height > capabilities->maxImageExtent.height)
        actual_extent.height = capabilities->maxImageExtent.height;

    return actual_extent;
}

static int vk_create_swapchain(int window_id)
{
    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    SwapchainSupportDetails support = vk_query_swapchain_support(vk_info.physical_device, vk_win->surface);

    if (support.format_count == 0 || support.present_mode_count == 0)
    {
        vk_cleanup_swapchain_support_details(&support);
        CANVAS_ERR("inadequate swapchain support\n");
        return CANVAS_FAIL;
    }

    VkSurfaceFormatKHR surface_format = vk_choose_surface_format(support.formats, support.format_count);
    VkPresentModeKHR present_mode = vk_choose_present_mode(support.present_modes, support.present_mode_count, canvas_info.canvas[window_id].vsync);
    VkExtent2D extent = vk_choose_swap_extent(&support.capabilities, window_id);

    uint32_t image_count = support.capabilities.minImageCount + 1;

    if (support.capabilities.maxImageCount > 0 && image_count > support.capabilities.maxImageCount)
        image_count = support.capabilities.maxImageCount;

    if (image_count > MAX_SWAPCHAIN_IMAGES)
        image_count = MAX_SWAPCHAIN_IMAGES;

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

    vk_info.vkGetSwapchainImagesKHR(vk_info.device, vk_win->swapchain, &vk_win->swapchain_image_count, NULL);

    if (vk_win->swapchain_image_count > MAX_SWAPCHAIN_IMAGES)
        vk_win->swapchain_image_count = MAX_SWAPCHAIN_IMAGES;

    vk_info.vkGetSwapchainImagesKHR(vk_info.device, vk_win->swapchain, &vk_win->swapchain_image_count, vk_win->swapchain_images);

    vk_win->swapchain_format = surface_format.format;
    vk_win->swapchain_extent = extent;

    CANVAS_VERBOSE("swapchain created: %ux%u, %u images\n", extent.width, extent.height, vk_win->swapchain_image_count);

    return CANVAS_OK;
}

static int vk_create_image_views(int window_id)
{
    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    for (uint32_t i = 0; i < vk_win->swapchain_image_count; i++)
    {
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

            return CANVAS_FAIL;
        }
    }

    return CANVAS_OK;
}

static int vk_create_render_pass(int window_id)
{
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

    return CANVAS_OK;
}

static int vk_create_framebuffers(int window_id)
{
    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    for (uint32_t i = 0; i < vk_win->swapchain_image_count; i++)
    {
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

            return CANVAS_FAIL;
        }
    }

    return CANVAS_OK;
}

static int vk_create_command_pool(int window_id)
{
    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    VkCommandPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = (uint32_t)vk_info.graphics_family;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult result = vk_info.vkCreateCommandPool(vk_info.device, &pool_info, NULL, &vk_win->command_pool);
    VK_CHECK(result, "failed to create command pool");

    return CANVAS_OK;
}

static int vk_create_command_buffers(int window_id)
{
    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = vk_win->command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = vk_win->swapchain_image_count;

    VkResult result = vk_info.vkAllocateCommandBuffers(vk_info.device, &alloc_info, vk_win->command_buffers);
    VK_CHECK(result, "failed to allocate command buffers");

    return CANVAS_OK;
}

static int vk_create_sync_objects(int window_id)
{
    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    VkSemaphoreCreateInfo semaphore_info = {0};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {0};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkResult result;

        result = vk_info.vkCreateSemaphore(vk_info.device, &semaphore_info, NULL, &vk_win->image_available_semaphores[i]);
        if (result != VK_SUCCESS)
        {
            CANVAS_ERR("failed to create image available semaphore %d (result=%d)\n", i, result);
            goto cleanup;
        }

        result = vk_info.vkCreateSemaphore(vk_info.device, &semaphore_info, NULL, &vk_win->render_finished_semaphores[i]);
        if (result != VK_SUCCESS)
        {
            CANVAS_ERR("failed to create render finished semaphore %d (result=%d)\n", i, result);
            vk_info.vkDestroySemaphore(vk_info.device, vk_win->image_available_semaphores[i], NULL);
            goto cleanup;
        }

        result = vk_info.vkCreateFence(vk_info.device, &fence_info, NULL, &vk_win->in_flight_fences[i]);
        if (result != VK_SUCCESS)
        {
            CANVAS_ERR("failed to create fence %d (result=%d)\n", i, result);
            vk_info.vkDestroySemaphore(vk_info.device, vk_win->image_available_semaphores[i], NULL);
            vk_info.vkDestroySemaphore(vk_info.device, vk_win->render_finished_semaphores[i], NULL);
            goto cleanup;
        }
    }

    for (uint32_t i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
        vk_win->images_in_flight[i] = VK_NULL_HANDLE;

    return CANVAS_OK;

cleanup:
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vk_win->image_available_semaphores[i])
            vk_info.vkDestroySemaphore(vk_info.device, vk_win->image_available_semaphores[i], NULL);

        if (vk_win->render_finished_semaphores[i])
            vk_info.vkDestroySemaphore(vk_info.device, vk_win->render_finished_semaphores[i], NULL);

        if (vk_win->in_flight_fences[i])
            vk_info.vkDestroyFence(vk_info.device, vk_win->in_flight_fences[i], NULL);
    }
    return CANVAS_FAIL;
}

static void vk_cleanup_swapchain(int window_id)
{
    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    if (!vk_win->initialized)
        return;

    vk_info.vkDeviceWaitIdle(vk_info.device);

    if (vk_win->command_pool && vk_win->swapchain_image_count > 0)
    {
        vk_info.vkFreeCommandBuffers(vk_info.device, vk_win->command_pool, vk_win->swapchain_image_count, vk_win->command_buffers);

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
}

static int vk_recreate_swapchain(int window_id)
{
    return CANVAS_OK; // Temporary, got a crash here

    int width = canvas_info.canvas[window_id].width;
    int height = canvas_info.canvas[window_id].height;

    while (width == 0 || height == 0)
    {
        canvas_sleep(0.01);
        width = canvas_info.canvas[window_id].width;
        height = canvas_info.canvas[window_id].height;
    }

    vk_info.vkDeviceWaitIdle(vk_info.device);

    vk_cleanup_swapchain(window_id);

    int result;
    result = vk_create_swapchain(window_id);
    if (result != CANVAS_OK)
    {
        CANVAS_ERR("vk_recreate_swapchain: Failed to create swapchain\n");
        return result;
    }

    result = vk_create_image_views(window_id);
    if (result != CANVAS_OK)
    {
        CANVAS_ERR("vk_recreate_swapchain: Failed to create image views\n");
        return result;
    }

    result = vk_create_framebuffers(window_id);
    if (result != CANVAS_OK)
    {
        CANVAS_ERR("vk_recreate_swapchain: Failed to create framebuffers\n");
        return result;
    }

    result = vk_create_command_buffers(window_id);
    if (result != CANVAS_OK)
    {
        CANVAS_ERR("vk_recreate_swapchain: Failed to create command buffers\n");
        return result;
    }

    vk_windows[window_id].needs_resize = false;

    return CANVAS_OK;
}

static int vk_record_command_buffer(int window_id, uint32_t image_index)
{
    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkResult result = vk_info.vkBeginCommandBuffer(vk_win->command_buffers[image_index], &begin_info);
    if (result != VK_SUCCESS)
    {
        CANVAS_ERR("failed to begin command buffer (result=%d)\n", result);
        return CANVAS_FAIL;
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
        CANVAS_ERR("failed to end command buffer (result=%d)\n", result);
        return CANVAS_FAIL;
    }

    return CANVAS_OK;
}

static int vk_draw_frame(int window_id)
{
    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    if (!vk_win->initialized)
        return CANVAS_OK;

    if (vk_win->current_frame >= MAX_FRAMES_IN_FLIGHT)
        vk_win->current_frame = 0;

    VkFence current_fence = vk_win->in_flight_fences[vk_win->current_frame];

    if (current_fence == VK_NULL_HANDLE)
        return CANVAS_FAIL;

    vk_info.vkWaitForFences(vk_info.device, 1, &current_fence, VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vk_info.vkAcquireNextImageKHR(vk_info.device, vk_win->swapchain, UINT64_MAX, vk_win->image_available_semaphores[vk_win->current_frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        int recreate_result = vk_recreate_swapchain(window_id);
        return recreate_result;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        CANVAS_ERR("failed to acquire swapchain image\n");
        return CANVAS_FAIL;
    }

    if (image_index >= vk_win->swapchain_image_count)
        return CANVAS_FAIL;

    if (vk_win->images_in_flight[image_index] != VK_NULL_HANDLE)
    {
        vk_info.vkWaitForFences(vk_info.device, 1, &vk_win->images_in_flight[image_index], VK_TRUE, UINT64_MAX);
    }

    vk_win->images_in_flight[image_index] = current_fence;

    int record_result = vk_record_command_buffer(window_id, image_index);

    if (record_result != CANVAS_OK)
        return record_result;

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {vk_win->image_available_semaphores[vk_win->current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &vk_win->command_buffers[image_index];

    VkSemaphore signal_semaphores[] = {vk_win->render_finished_semaphores[vk_win->current_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    vk_info.vkResetFences(vk_info.device, 1, &vk_win->in_flight_fences[vk_win->current_frame]);

    result = vk_info.vkQueueSubmit(vk_info.graphics_queue, 1, &submit_info, vk_win->in_flight_fences[vk_win->current_frame]);
    if (result != VK_SUCCESS)
    {
        CANVAS_ERR("failed to submit draw command buffer\n");
        return CANVAS_FAIL;
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

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vk_win->needs_resize)
    {
        vk_win->needs_resize = false;
        int recreate_result = vk_recreate_swapchain(window_id);
        return recreate_result;
    }
    else if (result != VK_SUCCESS)
    {
        CANVAS_ERR("failed to present swapchain image\n");
        return CANVAS_FAIL;
    }

    vk_win->current_frame = (vk_win->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    return CANVAS_OK;
}

static void vk_cleanup_window(int window_id)
{
    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    if (!vk_win->initialized)
        return;

    vk_info.vkDeviceWaitIdle(vk_info.device);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vk_win->image_available_semaphores[i])
            vk_info.vkDestroySemaphore(vk_info.device, vk_win->image_available_semaphores[i], NULL);

        if (vk_win->render_finished_semaphores[i])
            vk_info.vkDestroySemaphore(vk_info.device, vk_win->render_finished_semaphores[i], NULL);

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
}

static void vk_cleanup(void)
{
    if (!vk_info.instance)
        return;

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
}

#endif // CANVAS_VULKAN

//
//
//
#if defined(__APPLE__)

int canvas_minimize(int window_id)
{
    CANVAS_VALID(window_id);

    objc_id window = canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to minimize: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    msg_void_id(window, sel_c("miniaturize:"), NULL);

    canvas_info.canvas[window_id].minimized = true;
    canvas_info.canvas[window_id].maximized = false;

    return CANVAS_OK;
}

int canvas_maximize(int window_id)
{
    CANVAS_VALID(window_id);

    objc_id window = canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to maximize: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    bool is_zoomed = msg_bool(window, "isZoomed");

    if (!is_zoomed)
    {
        msg_void_id(window, sel_c("zoom:"), NULL);
        canvas_info.canvas[window_id].maximized = true;
    }

    canvas_info.canvas[window_id].minimized = false;

    return CANVAS_OK;
}

int canvas_restore(int window_id)
{
    CANVAS_VALID(window_id);

    objc_id window = canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to restore: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    if (canvas_info.canvas[window_id].minimized)
    {
        msg_void_id(window, sel_c("deminiaturize:"), NULL);
    }
    else if (canvas_info.canvas[window_id].maximized)
    {
        msg_void_id(window, sel_c("zoom:"), NULL);
    }

    canvas_info.canvas[window_id].minimized = false;
    canvas_info.canvas[window_id].maximized = false;

    return CANVAS_OK;
}

static objc_id _canvas_get_ns_cursor(canvas_cursor_type cursor)
{
    if (cursor < 0 || cursor >= sizeof(_cursor_selector_names) / sizeof(_cursor_selector_names[0]))
        cursor = ARROW;

    const char *selector = _cursor_selector_names[cursor];

    if (!selector)
        selector = "arrowCursor";

    return msg_id(cls("NSCursor"), selector);
}

int canvas_cursor(int window_id, canvas_cursor_type cursor)
{
    CANVAS_VALID(window_id);

    canvas_info.canvas[window_id].cursor = cursor;

    objc_id ns_cursor = _canvas_get_ns_cursor(cursor);
    if (ns_cursor)
        msg_void(ns_cursor, "set");

    return CANVAS_OK;
}

int _canvas_close(int window_id)
{
    CANVAS_BOUNDS(window_id);

    objc_id window = canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to close: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    _canvas_data[window_id].layer = NULL;
    _canvas_data[window_id].view = NULL;
    _canvas_data[window_id].scale = 0.0;

    msg_void(window, "close");
    msg_void(window, "release");

    return CANVAS_OK;
}

int _canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title)
{
    CANVAS_BOUNDS(window_id);

    objc_id window = canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to set: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
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

    return CANVAS_OK;
}

int _canvas_get_window_display(int window_id)
{
    CANVAS_BOUNDS(window_id);

    objc_id window = canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to get display: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    objc_id screen = msg_id(window, "screen");
    if (!screen)
    {
        CANVAS_ERR("no screen for window: %d\n", window_id);
        return CANVAS_ERR_GET_DISPLAY;
    }

    objc_id device_desc = msg_id(screen, "deviceDescription");
    if (!device_desc)
    {
        CANVAS_ERR("no device description: %d\n", window_id);
        return CANVAS_ERR_GET_GPU;
    }

    objc_id screen_number_key = nsstring_from_cstr("NSScreenNumber");
    if (!screen_number_key)
        return CANVAS_ERR_GET_DISPLAY;

    objc_id display_id_obj = msg_id_id(device_desc, "objectForKey:", screen_number_key);
    if (!display_id_obj)
        return CANVAS_ERR_GET_DISPLAY;

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
        return i;
    }

    return CANVAS_OK;
}

int _canvas_refresh_displays()
{
    canvas_info.display_count = 0;
    canvas_info.display_changed = false;

    uint32_t max_displays = MAX_DISPLAYS;
    CGDirectDisplayID display[MAX_DISPLAYS];
    uint32_t display_count = 0;

    CGError err = CGGetActiveDisplayList(max_displays, display, &display_count);
    if (err != kCGErrorSuccess)
    {
        CANVAS_ERR("get display list: %d\n", err);
        return CANVAS_ERR_GET_DISPLAY;
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

    return canvas_info.display_count;
}

void _canvas_display_reconfigure_callback(CGDirectDisplayID display, CGDisplayChangeSummaryFlags flags, void *userInfo)
{
    if (flags & (kCGDisplayAddFlag | kCGDisplayRemoveFlag | kCGDisplaySetModeFlag | kCGDisplayDesktopShapeChangedFlag))
        canvas_info.display_changed = true;
}

int _canvas_init_displays()
{
    canvas_info.display_count = 0;
    CGDisplayRegisterReconfigurationCallback(_canvas_display_reconfigure_callback, NULL);

    return _canvas_refresh_displays();
}

void _post_init()
{
    if (canvas_info.init_post)
        return;
    canvas_info.init_post = 1;

    objc_id poolClass = cls("NSAutoreleasePool");
    if (!poolClass)
        return;

    objc_id alloc = msg_id(poolClass, "alloc");
    canvas_macos.pool = msg_id(alloc, "init");
    mach_timebase_info(&canvas_macos.timebase);
}

int _canvas_platform()
{
    _post_init();

    canvas_macos.app = msg_id(cls("NSApplication"), "sharedApplication");
    if (!canvas_macos.app)
    {
        CANVAS_ERR("failed to get NSApplication\n");
        return CANVAS_ERR_GET_PLATFORM;
    }

    msg_void_long(canvas_macos.app, "setActivationPolicy:", NSApplicationActivationPolicyRegular);
    msg_void_bool(canvas_macos.app, "activateIgnoringOtherApps:", true);

    return CANVAS_OK;
}

int _canvas_window(int x, int y, int width, int height, const char *title)
{
    _post_init();
    canvas_startup();

    unsigned long style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;

    objc_id winClass = cls("NSWindow");
    if (!winClass)
    {
        CANVAS_ERR("failed to get NSWindow class\n");
        return CANVAS_ERR_GET_WINDOW;
    }

    int window_id = _canvas_get_free();
    if (window_id < 0)
        return window_id;

    objc_id walloc = msg_id(winClass, "alloc");

    typedef objc_id (*msg_initWin)(objc_id, objc_sel, _CGRect, unsigned long, long, int);

    _CGRect rect = make_rect(x, y, width, height);

    objc_id window = ((msg_initWin)objc_msgSend)(walloc, sel_c("initWithContentRect:styleMask:backing:defer:"), rect, style, (long)NSBackingStoreBuffered, 0);

    if (!window)
    {
        canvas_info.canvas[window_id]._valid = false;
        CANVAS_ERR("failed to create NSWindow\n");
        return CANVAS_ERR_GET_WINDOW;
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
    canvas_info.canvas[window_id]._valid = true;

    return window_id;
}

int _canvas_gpu_init()
{
    if (canvas_info.init_gpu)
        return CANVAS_OK;
    canvas_info.init_gpu = 1;

    canvas_macos.device = MTLCreateSystemDefaultDevice();
    if (!canvas_macos.device)
    {
        CANVAS_ERR("failed to create Metal device\n");
        return CANVAS_ERR_GET_GPU;
    }

    canvas_macos.queue = msg_id(canvas_macos.device, "newCommandQueue");
    if (!canvas_macos.queue)
    {
        CANVAS_ERR("failed to create Metal command queue\n");
        return CANVAS_ERR_GET_GPU;
    }

    return CANVAS_OK;
}

int _canvas_update_drawable_size(int window_id)
{
    CANVAS_BOUNDS(window_id);

    if (!_canvas_data[window_id].view || !_canvas_data[window_id].layer)
    {
        CANVAS_ERR("no view or layer to update drawable size: %d\n", window_id);
        return CANVAS_ERR_GET_DISPLAY;
    }

    _CGRect b = msg_rect(_canvas_data[window_id].view, "bounds");
    double w = b.w * _canvas_data[window_id].scale;
    double h = b.h * _canvas_data[window_id].scale;
    msg_void_size(_canvas_data[window_id].layer, "setDrawableSize:", w, h);

    return CANVAS_OK;
}

int _canvas_gpu_new_window(int window_id)
{
    CANVAS_BOUNDS(window_id);

    objc_id window = canvas_info.canvas[window_id].window;

    objc_id view = msg_id(cls("NSView"), "alloc");
    view = msg_id_rect(view, "initWithFrame:", make_rect(0, 0, 800, 600));
    if (!view)
    {
        CANVAS_ERR("failed to create NSView for window: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    msg_void_bool(view, "setWantsLayer:", true);

    objc_id layer = msg_id(cls("CAMetalLayer"), "alloc");
    layer = msg_id(layer, "init");
    if (!layer)
    {
        CANVAS_ERR("failed to create CAMetalLayer for window: %d\n", window_id);
        return CANVAS_ERR_GET_GPU;
    }

    msg_void_id(layer, "setDevice:", canvas_macos.device);
    msg_void_long(layer, "setPixelFormat:", MTLPixelFormatBGRA8Unorm);
    msg_void_bool(layer, "setFramebufferOnly:", true);
    msg_void_bool(layer, "setAllowsNextDrawableTimeout:", false);

    double scale = 1.0;
    objc_id screen = msg_id(window, "screen");
    if (screen)
        scale = msg_dbl(screen, "backingScaleFactor");
    msg_void_double(layer, "setContentsScale:", scale);

    msg_void_id(view, "setLayer:", layer);
    msg_void_id(window, "setContentView:", view);

    _canvas_data[window_id].view = view;
    _canvas_data[window_id].layer = layer;
    _canvas_data[window_id].scale = scale;

    return CANVAS_OK;
}

void _canvas_gpu_draw_all()
{
    if (!canvas_macos.queue)
    {
        CANVAS_VERBOSE("no metal command queue\n");
        return;
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
}

int _canvas_update()
{
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
                if (eventType == 5) // NSEventTypeMouseMoved
                {
                    objc_id ns_cursor = _canvas_get_ns_cursor(canvas_info.canvas[window_idx].cursor);
                    if (ns_cursor)
                        msg_void(ns_cursor, "set");
                }

                switch (eventType)
                {
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

    return CANVAS_OK;
}

int _canvas_post_update()
{
    return CANVAS_OK;
}

int _canvas_exit()
{
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
    CGDisplayRemoveReconfigurationCallback(_canvas_display_reconfigure_callback, NULL);
    return CANVAS_OK;
}

static inline void _canvas_ensure_timebase()
{
    if (canvas_macos.timebase.denom == 0)
        mach_timebase_info(&canvas_macos.timebase);
}

double canvas_get_time(canvas_time_data *time)
{
    _canvas_ensure_timebase();
    uint64_t elapsed = mach_absolute_time() - time->start;
    return (double)elapsed * (double)canvas_macos.timebase.numer /
           (double)canvas_macos.timebase.denom / 1e9;
}

void canvas_sleep(double seconds)
{
    struct timespec ts;
    ts.tv_sec = (time_t)seconds;
    ts.tv_nsec = (long)((seconds - (double)ts.tv_sec) * 1e9);
    nanosleep(&ts, NULL);
}

void canvas_time_init(canvas_time_data *time)
{
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
}

#endif /* __APPLE__ */

//
//
//
#if defined(_WIN32) || defined(_WIN64)

int canvas_minimize(int window_id)
{
    CANVAS_VALID(window_id);

    HWND window = (HWND)canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to minimize: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    ShowWindow(window, SW_MINIMIZE);
    canvas_info.canvas[window_id].minimized = true;
    canvas_info.canvas[window_id].maximized = false;

    return CANVAS_OK;
}

int canvas_maximize(int window_id)
{
    CANVAS_VALID(window_id);

    HWND window = (HWND)canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to maximize: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    ShowWindow(window, SW_MAXIMIZE);
    canvas_info.canvas[window_id].maximized = true;
    canvas_info.canvas[window_id].minimized = false;

    return CANVAS_OK;
}

int canvas_restore(int window_id)
{
    CANVAS_VALID(window_id);

    HWND window = (HWND)canvas_info.canvas[window_id].window;
    if (!window)
    {
        CANVAS_ERR("no window to restore: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
    }

    ShowWindow(window, SW_RESTORE);
    canvas_info.canvas[window_id].minimized = false;
    canvas_info.canvas[window_id].maximized = false;

    return CANVAS_OK;
}

int _canvas_close(int window_id)
{
    CANVAS_BOUNDS(window_id);

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
    return CANVAS_OK;
}

int _canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title)
{
    CANVAS_BOUNDS(window_id);

    HWND window = (HWND)canvas_info.canvas[window_id].window;

    if (!window)
    {
        CANVAS_ERR("no window to set: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
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

    return CANVAS_OK;
}

int _canvas_get_window_display(int window_id)
{
    CANVAS_BOUNDS(window_id);

    HWND window = (HWND)canvas_info.canvas[window_id].window;

    if (!window)
    {
        CANVAS_ERR("no window to set: %d\n", window_id);
        return CANVAS_ERR_GET_WINDOW;
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
        return i;
    }

    return CANVAS_OK;
}

int _canvas_refresh_displays()
{
    canvas_info.display_count = 0;
    canvas_info.display_changed = false;

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

    return canvas_info.display_count;
}

int _canvas_init_displays()
{
    canvas_info.display_count = 0;

    // SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    return _canvas_refresh_displays();
}

double canvas_get_time(canvas_time_data *time)
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

void canvas_time_init(canvas_time_data *time)
{
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
}

HCURSOR _canvas_get_win32_cursor(canvas_cursor_type cursor)
{
    LPCTSTR cursor_name;

    switch (cursor)
    {
    case ARROW:
        cursor_name = IDC_ARROW;
        break;
    case TEXT:
        cursor_name = IDC_IBEAM;
        break;
    case CROSSHAIR:
        cursor_name = IDC_CROSS;
        break;
    case HAND:
        cursor_name = IDC_HAND;
        break;
    case SIZE_NS:
        cursor_name = IDC_SIZENS;
        break;
    case SIZE_EW:
        cursor_name = IDC_SIZEWE;
        break;
    case SIZE_NESW:
        cursor_name = IDC_SIZENESW;
        break;
    case SIZE_NWSE:
        cursor_name = IDC_SIZENWSE;
        break;
    case SIZE_ALL:
        cursor_name = IDC_SIZEALL;
        break;
    case NOT_ALLOWED:
        cursor_name = IDC_NO;
        break;
    case WAIT:
        cursor_name = IDC_WAIT;
        break;
    default:
        cursor_name = IDC_ARROW;
        break;
    }

    return LoadCursor(NULL, cursor_name);
}

int canvas_cursor(int window_id, canvas_cursor_type cursor)
{
    CANVAS_VALID(window_id);

    canvas_info.canvas[window_id].cursor = cursor;
    SetCursor(_canvas_get_win32_cursor(cursor));

    return CANVAS_OK;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int window_index = _canvas_window_index(hwnd);

    if (window_index < 0)
        return DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg)
    {
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

            DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));
        }
        return CANVAS_OK;
    }

    case WM_NCCALCSIZE:
    {
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
            canvas_info.canvas[window_index].resize = true;

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
        SetTimer(hwnd, 1, 0, NULL); // 60 fps hard cap due to Windows...
        return CANVAS_OK;
    }

    case WM_EXITSIZEMOVE:
    {
        if (!canvas_info.os_timed)
            break;

        canvas_info.os_timed = false;
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

    case WM_CLOSE:
    {
        canvas_info.canvas[window_index].close = true;
        canvas_info.os_timed = false;

        if (canvas_info.os_timed)
            KillTimer(hwnd, 1);

        return 0;
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

    DWORD style = WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_VISIBLE;

    canvas_info.canvas[window_id].index = window_id;

    HWND window = CreateWindowA(
        "CanvasWindowClass",
        title,
        style,
        x, y, width, height,
        NULL, NULL, canvas_win32.instance, NULL);

    if (!window)
    {
        CANVAS_ERR("create win32 window");
        return CANVAS_ERR_GET_WINDOW;
    }

    canvas_info.canvas[window_id].window = window;
    canvas_info.canvas[window_id].resize = false;
    canvas_info.canvas[window_id].titlebar = false;
    canvas_info.canvas[window_id]._valid = true;

    return window_id;
}

int _canvas_platform()
{
    canvas_win32.instance = GetModuleHandle(NULL);

    WNDCLASSA wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = canvas_win32.instance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "CanvasWindowClass";

    canvas_win32.class = RegisterClassA(&wc);

    if (!canvas_win32.class)
    {
        CANVAS_ERR("register windows class failed");
        return CANVAS_ERR_GET_PLATFORM;
    }

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

    return CANVAS_OK;
}

int _canvas_gpu_init()
{
    if (canvas_info.init_gpu)
        return CANVAS_OK;

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
    return CANVAS_ERR_GET_GPU;
}

int _canvas_gpu_new_window(int window_id)
{
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
        CANVAS_ERR("query swapchain interface\n");
        return CANVAS_ERR_GET_GPU;
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
            CANVAS_ERR("get swapchain buffer %d\n", i);
            return CANVAS_ERR_GET_GPU;
        }

        _canvas_data[window_id].rtvHandles[i] = rtvHandle;
        canvas_win32.device->lpVtbl->CreateRenderTargetView(
            canvas_win32.device,
            _canvas_data[window_id].backBuffers[i],
            NULL,
            rtvHandle);
        rtvHandle.ptr += canvas_win32.rtvDescriptorSize;
    }

    return CANVAS_OK;
}

int _canvas_window_resize(int window_id)
{
    CANVAS_BOUNDS(window_id);

    canvas_data *window = &_canvas_data[window_id];

    if (!window->swapChain || !canvas_info.canvas[window_id].resize)
    {
        canvas_info.canvas[window_id].resize = false;
        return CANVAS_OK;
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
    canvas_win32.rtvHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(canvas_win32.rtvHeap, &rtvHandle);
    rtvHandle.ptr += window_id * 2 * canvas_win32.rtvDescriptorSize;

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
        canvas_win32.device->lpVtbl->CreateRenderTargetView(
            canvas_win32.device, window->backBuffers[i], NULL, rtvHandle);
        rtvHandle.ptr += canvas_win32.rtvDescriptorSize;
    }

    return CANVAS_OK;
}

int _canvas_post_update()
{
    return CANVAS_OK;
}

int _canvas_exit()
{
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
    return CANVAS_OK;
}

#endif

//
//
//
#if defined(__linux__)

int canvas_minimize(int window_id)
{
    CANVAS_VALID(window_id);

    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!x11.display)
        {
            CANVAS_ERR("no display connection\n");
            return CANVAS_ERR_GET_DISPLAY;
        }

        Window window = (Window)canvas_info.canvas[window_id].window;
        if (!window)
        {
            CANVAS_ERR("no window to minimize: %d\n", window_id);
            return CANVAS_ERR_GET_WINDOW;
        }

        int screen = x11.XDefaultScreen(x11.display);
        x11.XIconifyWindow(x11.display, window, screen);
        x11.XFlush(x11.display);
    }

    canvas_info.canvas[window_id].minimized = true;
    canvas_info.canvas[window_id].maximized = false;

    return CANVAS_OK;
}

int canvas_maximize(int window_id)
{
    CANVAS_VALID(window_id);

    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!x11.display)
        {
            CANVAS_ERR("no display connection\n");
            return CANVAS_ERR_GET_DISPLAY;
        }

        Window window = (Window)canvas_info.canvas[window_id].window;
        if (!window)
        {
            CANVAS_ERR("no window to maximize: %d\n", window_id);
            return CANVAS_ERR_GET_WINDOW;
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

    return CANVAS_OK;
}

int canvas_restore(int window_id)
{
    CANVAS_VALID(window_id);

    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!x11.display)
        {
            CANVAS_ERR("no display connection\n");
            return CANVAS_ERR_GET_DISPLAY;
        }

        Window window = (Window)canvas_info.canvas[window_id].window;
        if (!window)
        {
            CANVAS_ERR("no window to restore: %d\n", window_id);
            return CANVAS_ERR_GET_WINDOW;
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

        x11.XFlush(x11.display);
    }

    canvas_info.canvas[window_id].minimized = false;
    canvas_info.canvas[window_id].maximized = false;

    return CANVAS_OK;
}

int _canvas_close(int window_id)
{
    CANVAS_BOUNDS(window_id);

    if (_canvas_using_wayland)
    {
    }
    else
    {
        Window window = (Window)canvas_info.canvas[window_id].window;

        if (!window)
        {
            CANVAS_ERR("no window to close: %d\n", window_id);
            return CANVAS_ERR_GET_WINDOW;
        }

        x11.XDestroyWindow(x11.display, window);
        x11.XFlush(x11.display);
    }

    return CANVAS_OK;
}

int _canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title)
{
    CANVAS_VALID(window_id);
    CANVAS_DISPLAY_BOUNDS(display);

    if (_canvas_using_wayland)
    {
    }
    else
    {
        Window window = (Window)canvas_info.canvas[window_id].window;

        if (title)
        {
            x11.XStoreName(x11.display, window, title);
            Atom net_wm_name = x11.XInternAtom(x11.display, "_NET_WM_NAME", 0);
            Atom utf8_string = x11.XInternAtom(x11.display, "UTF8_STRING", 0);
            x11.XChangeProperty(x11.display, window, net_wm_name, utf8_string, 8, 0, (unsigned char *)title, strlen(title));
        }

        _canvas_data[window_id].client_set = true;

        int global_x = canvas_info.display[display].x + x;
        int global_y = canvas_info.display[display].y + y;

        x11.XMoveResizeWindow(x11.display, window, global_x, global_y, width, height);
    }

    canvas_info.canvas[window_id].os_moved = false;
    canvas_info.canvas[window_id].os_resized = false;

    return CANVAS_OK;
}

int _canvas_init_displays()
{
    canvas_info.display_count = 1;

    if (_canvas_using_wayland)
    {
        return canvas_info.display_count;
    }

    if (xrandr.library)
    {
        Window root = x11.XDefaultRootWindow(x11.display);
        XRRScreenResources *sr = xrandr.XRRGetScreenResourcesCurrent(x11.display, root);

        if (!sr)
        {
            CANVAS_WARN("XRRGetScreenResourcesCurrent failed\n");
            dlclose(xrandr.library);
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
        dlclose(xrandr.library);

        return canvas_info.display_count;
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

    return canvas_info.display_count;
}

int _canvas_get_window_display(int window_id)
{
    CANVAS_BOUNDS(window_id);

    if (_canvas_using_wayland)
    {
    }
    else
    {
    }

    return CANVAS_OK;
}

int _canvas_get_resize_edge_action(int window_id, int x, int y)
{
    int width = canvas_info.canvas[window_id].width;
    int height = canvas_info.canvas[window_id].height;
    int border = 8;

    bool at_left = x < border;
    bool at_right = x > width - border;
    bool at_top = y < border;
    bool at_bottom = y > height - border;

    if (at_top && at_left)
        return _NET_WM_MOVERESIZE_SIZE_TOPLEFT;
    if (at_top && at_right)
        return _NET_WM_MOVERESIZE_SIZE_TOPRIGHT;
    if (at_bottom && at_left)
        return _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT;
    if (at_bottom && at_right)
        return _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT;

    if (at_top)
        return _NET_WM_MOVERESIZE_SIZE_TOP;
    if (at_bottom)
        return _NET_WM_MOVERESIZE_SIZE_BOTTOM;
    if (at_left)
        return _NET_WM_MOVERESIZE_SIZE_LEFT;
    if (at_right)
        return _NET_WM_MOVERESIZE_SIZE_RIGHT;

    return -1;
}

void _canvas_start_wm_move_resize(int window_id, int x_root, int y_root, int action)
{
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
}

int _canvas_window(int x, int y, int width, int height, const char *title)
{
    canvas_startup();

    canvas_window_handle window = 0;

    int window_id = _canvas_get_free();
    if (window_id < 0)
        return window_id;

    canvas_info.canvas[window_id] = (canvas_type){0};
    _canvas_data[window_id] = (canvas_data){0};

    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!x11.display)
        {
            CANVAS_ERR("no display connection for window creation\n");
            return CANVAS_ERR_GET_DISPLAY;
        }

        window = (canvas_window_handle)x11.XCreateSimpleWindow(
            x11.display,
            x11.XDefaultRootWindow(x11.display),
            x, y, width, height, 0,
            x11.XBlackPixel(x11.display, 0),
            x11.XWhitePixel(x11.display, 0));

        if (!window)
        {
            CANVAS_ERR("create x11 window\n");
            return CANVAS_ERR_GET_WINDOW;
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
    canvas_info.canvas[window_id]._valid = true;

    return window_id;
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
    case ARROW:
        return 2; // XC_arrow
    case TEXT:
        return 152; // XC_xterm
    case CROSSHAIR:
        return 34; // XC_crosshair
    case HAND:
        return 58; // XC_hand2
    case SIZE_NS:
        return 116; // XC_sb_v_double_arrow
    case SIZE_EW:
        return 108; // XC_sb_h_double_arrow
    case SIZE_NESW:
        return 52;
    case SIZE_NWSE:
        return 52;
    case SIZE_ALL:
        return 52; // XC_fleur
    case NOT_ALLOWED:
        return 0; // XC_X_cursor
    case WAIT:
        return 150; // XC_watch
    default:
        return 2; // XC_arrow
    }
}

int canvas_cursor(int window_id, canvas_cursor_type cursor)
{
    CANVAS_VALID(window_id);

    if (_canvas_using_wayland)
        return CANVAS_OK;

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

    return CANVAS_OK;
}

static void _canvas_set_active_cursor(int window_id, canvas_cursor_type cursor)
{
    if (canvas_info.canvas[window_id].active_cursor == cursor)
        return;

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
}

int _canvas_init_x11()
{

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
        CANVAS_ERR("open x11 display\n");
        return CANVAS_ERR_GET_DISPLAY;
    }

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

    return CANVAS_OK;
}

int _canvas_platform()
{
    if (canvas_info.init)
        return CANVAS_OK;

    //     int load_result = _canvas_init_wayland();

    if (!_canvas_using_wayland && _canvas_init_x11() < 0)
        return CANVAS_ERR_GET_DISPLAY;

    return CANVAS_OK;
}

static canvas_cursor_type _canvas_get_resize_cursor(int action)
{
    switch (action)
    {
    case _NET_WM_MOVERESIZE_SIZE_TOPLEFT:
        return SIZE_NESW;
    case _NET_WM_MOVERESIZE_SIZE_TOPRIGHT:
        return SIZE_NWSE;
    case _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT:
        return SIZE_NESW;
    case _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT:
        return SIZE_NWSE;
    case _NET_WM_MOVERESIZE_SIZE_TOP:
    case _NET_WM_MOVERESIZE_SIZE_BOTTOM:
        return SIZE_NS;
    case _NET_WM_MOVERESIZE_SIZE_LEFT:
    case _NET_WM_MOVERESIZE_SIZE_RIGHT:
        return SIZE_EW;
    default:
        return ARROW;
    }
}

static bool _canvas_is_window_maximized(int window_id)
{
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
        return false;

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

    return horz_maximized && vert_maximized;
}

int _canvas_update()
{
    if (_canvas_using_wayland)
    {
    }
    else
    {
        if (!x11.display)
        {
            CANVAS_VERBOSE("no display connection for update\n");
            return CANVAS_ERR_GET_DISPLAY;
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

                if (canvas_info.canvas[window_id].width != xce->width || canvas_info.canvas[window_id].height != xce->height)
                    canvas_info.canvas[window_id].os_resized = true;

                canvas_info.canvas[window_id].width = xce->width;
                canvas_info.canvas[window_id].height = xce->height;

                break;
            }

            case X11_ButtonPress:
            {
                XButtonEvent *xbe = (XButtonEvent *)&event;

                if (xbe->button == 1)
                {
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

            case X11_MotionNotify:
            {
                XMotionEvent *xme = (XMotionEvent *)&event;

                if (!canvas_info.canvas[window_id].maximized)
                {
                    int action = _canvas_get_resize_edge_action(window_id, xme->x, xme->y);

                    if (action >= 0)
                    {
                        canvas_cursor_type resize_cursor = _canvas_get_resize_cursor(action);
                        _canvas_set_active_cursor(window_id, resize_cursor);
                    }
                    else if (xme->y < 30)
                    {
                        _canvas_set_active_cursor(window_id, ARROW);
                    }
                    else
                    {
                        _canvas_set_active_cursor(window_id, canvas_info.canvas[window_id].cursor);
                    }
                }
                else
                {
                    if (xme->y < 30)
                    {
                        _canvas_set_active_cursor(window_id, ARROW);
                    }
                    else
                    {
                        _canvas_set_active_cursor(window_id, canvas_info.canvas[window_id].cursor);
                    }
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
                        canvas_info.canvas[window_id].active_cursor = ARROW;
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

            case X11_MapNotify:
            {
                canvas_info.canvas[window_id].minimized = false;
                break;
            }

            case X11_Expose:
            {
                break;
            }
            }
        }
    }

    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (!canvas_info.canvas[i]._valid || !vk_windows[i].initialized)
            continue;

        vk_draw_frame(i);
    }

    return CANVAS_OK;
}

int _canvas_gpu_init(void)
{
    if (canvas_info.init_gpu)
        return CANVAS_OK;

    int result = canvas_backend_vulkan_init();
    if (result != CANVAS_OK)
        return result;

    canvas_info.init_gpu = 1;
    return CANVAS_OK;
}

int _canvas_gpu_new_window(int window_id)
{
    CANVAS_BOUNDS(window_id);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];
    memset(vk_win, 0, sizeof(canvas_vulkan_window));

    int result;

    result = vk_create_surface(window_id, &vk_win->surface);
    if (result != CANVAS_OK)
    {
        CANVAS_ERR("failed to create surface for window %d\n", window_id);
        return result;
    }

    if (!vk_info.device)
    {
        VkResult vk_result = vk_select_physical_device(vk_win->surface);
        if (vk_result != VK_SUCCESS)
        {
            CANVAS_ERR("failed to select physical device\n");
            vk_info.vkDestroySurfaceKHR(vk_info.instance, vk_win->surface, NULL);
            return CANVAS_ERR_GET_GPU;
        }

        result = vk_create_logical_device();
        if (result != CANVAS_OK)
        {
            vk_info.vkDestroySurfaceKHR(vk_info.instance, vk_win->surface, NULL);
            return result;
        }

        result = vk_load_device_functions();
        if (result != CANVAS_OK)
        {
            vk_info.vkDestroyDevice(vk_info.device, NULL);
            vk_info.vkDestroySurfaceKHR(vk_info.instance, vk_win->surface, NULL);
            return result;
        }
    }

    result = vk_create_swapchain(window_id);
    if (result != CANVAS_OK)
        goto cleanup;

    result = vk_create_image_views(window_id);
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
    return CANVAS_OK;

cleanup:
    vk_cleanup_window(window_id);
    return result;
}

int _canvas_window_resize(int window_id)
{
    CANVAS_BOUNDS(window_id);

    canvas_vulkan_window *vk_win = &vk_windows[window_id];

    if (!vk_win->initialized)
        return CANVAS_OK;

    vk_win->needs_resize = true;
    return CANVAS_OK;
}

int _canvas_post_update()
{
    if (_canvas_using_wayland)
    {
    }
    else
    {
        x11.XFlush(x11.display);
    }

    return CANVAS_OK;
}

int _canvas_exit()
{
    if (vk_info.library)
        vk_cleanup();

    if (x11.display)
        x11.XCloseDisplay(x11.display);

    if (x11.library)
        dlclose(x11.library);

    if (xrandr.library)
        dlclose(xrandr.library);

    return CANVAS_OK;
}

double canvas_get_time(canvas_time_data *time)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    if (ts.tv_sec < 0 || ts.tv_nsec < 0)
        return 0.0;

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

void canvas_time_init(canvas_time_data *time)
{
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
}

#endif // _linux_

int _canvas_primary_display_index(void)
{
    for (int i = 0; i < canvas_info.display_count; ++i)
        if (canvas_info.display[i].primary)
            return i;
    return 0;
}

int canvas_startup()
{
    if (canvas_info.init)
        return CANVAS_OK;

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
    }

    int result = _canvas_platform();
    if (result != CANVAS_OK)
    {
        CANVAS_ERR("platform initialization failed\n");
        return result;
    }

    result = _canvas_init_displays();
    if (result < 0)
    {
        CANVAS_ERR("display initialization failed\n");
        return result;
    }

    canvas_info.init = true;

    return CANVAS_OK;
}
void canvas_main_loop()
{
    canvas_time_update(&canvas_info.time);

    _canvas_update();

    bool any_alive = false;
    for (int i = 0; i < MAX_CANVAS; ++i)
    {
        if (!canvas_info.canvas[i]._valid)
            continue;

        any_alive = true;

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
        CANVAS_DBG("auto exit triggered\n");
        canvas_info.quit = 1;
    }

    canvas_limit_fps(&canvas_info.time, canvas_info.limit_fps);
}

int canvas_exit()
{
    CANVAS_INFO("quitting canvas");
    canvas_info.quit = 1;
    return _canvas_exit();
}

int canvas_run(canvas_update_callback default_callback)
{
    if (!canvas_info.init)
    {
        CANVAS_ERR("refusing run, initialization failed.");
        return CANVAS_FAIL;
    }

    canvas_info.update_callback = default_callback;

    while (!canvas_info.quit)
    {
        canvas_info.os_timed = false;
        canvas_main_loop();
    }

    return canvas_exit();
}

// display:     -1 = primary display
// x:           -1 = centered
// y:           -1 = centered
// width:       window width in pixels
// height:      window height in pixels
// title:       window title string
int canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title)
{
    CANVAS_VALID(window_id);

    if (canvas_info.display_count <= 0)
    {
        CANVAS_ERR("no displays available\n");
        return CANVAS_ERR_GET_DISPLAY;
    }

    if (display < 0 || display >= canvas_info.display_count)
    {
        display = _canvas_primary_display_index();
    }

    canvas_info.canvas[window_id].display = display;
    canvas_info.canvas[window_id].width = width;
    canvas_info.canvas[window_id].height = height;

    canvas_info.canvas[window_id].os_moved = false;
    canvas_info.canvas[window_id].os_resized = false;

    canvas_info.canvas[window_id].x = x;
    canvas_info.canvas[window_id].y = y;

    CANVAS_DISPLAY_BOUNDS(display);

    int target_x = (x == -1) ? canvas_info.display[display].width / 2 - width / 2 : x;
    int target_y = (y == -1) ? canvas_info.display[display].height / 2 - height / 2 : y;

    if (title && strlen(title) > 0)
    {
        strncpy(canvas_info.canvas[window_id].title, title, MAX_CANVAS_TITLE - 1);
        canvas_info.canvas[window_id].title[MAX_CANVAS_TITLE - 1] = '\0';
    }
    else
    {
        canvas_info.canvas[window_id].title[0] = '\0';
    }

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

    canvas_time_init(&canvas_info.canvas[result].time);

    _canvas_get_window_display(result);

    canvas_info.canvas[result].cursor = ARROW;

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
    CANVAS_BOUNDS(window_id);

    if (!canvas_info.canvas[window_id]._valid)
        return CANVAS_OK;

    canvas_info.canvas[window_id]._valid = false;

    _canvas_close(window_id);

    canvas_info.canvas[window_id] = (canvas_type){0};
    _canvas_data[window_id] = (canvas_data){0};

    return CANVAS_OK;
}

int canvas_set_update_callback(int window_id, canvas_update_callback callback)
{
    CANVAS_VALID(window_id);

    canvas_info.canvas[window_id].update = callback;
    return CANVAS_OK;
}

int canvas_color(int window_id, const float color[4])
{
    CANVAS_VALID(window_id);

    canvas_info.canvas[window_id].clear[0] = color[0];
    canvas_info.canvas[window_id].clear[1] = color[1];
    canvas_info.canvas[window_id].clear[2] = color[2];
    canvas_info.canvas[window_id].clear[3] = color[3];
    return CANVAS_OK;
}

//
//
// Time

void canvas_time_update(canvas_time_data *time)
{
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
}

int canvas_time_fixed_step(canvas_time_data *time, double fixed_dt, int max_steps)
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
}

#endif // CANVAS_HEADER_ONLY

CANVAS_EXTERN_C_END