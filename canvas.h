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
        MacOS                   \       \       Metal       -framework Cocoa
        Linux                   \       ~       Vulkan      -lX11
        iOS                     ~       ~       Metal
        Android                 ~       ~       Vulkan
        HTML5                   ~       ~       WebGPU

        * note for windows:
        x86_64-w64-mingw32-gcc for cross compiling to windows

        -  Building  -

        example with zig on macos:
        zig cc example/simple.c -framework Cocoa -I. -s -O3 -Qn -fno-ident -fno-unwind-tables -fno-asynchronous-unwind-tables -fomit-frame-pointer -fno-stack-protector -fno-stack-clash-protection

        example with zig on windows:
        zig cc example/simple.c -I. -s -O3 -Qn -mwindows -ldwmapi

        clang example/simple.c -framework Cocoa -I. -Oz -fno-ident -fno-unwind-tables -fno-asynchronous-unwind-tables -fomit-frame-pointer -fno-stack-protector -Wl,-dead_strip && ./a.out

        -  Info  -
        Canvas is created by Dawn Larsson 2025
        This is licensed under Apache License 2.0, by Dawn Larsson

        https://dawning.dev/  -  https://docs.dawning.dev/

        repo
        https://github.com/dawnlarsson/dawning-canvas

*/

#pragma once

#ifndef MAX_CANVAS
#define MAX_CANVAS 16
#endif

#ifndef MAX_DISPLAYS
#define MAX_DISPLAYS 8
#endif

#include <stdbool.h>
#include <stdint.h>

typedef void *canvas_window_handle;
typedef void (*canvas_update_callback)(int window);
canvas_update_callback canvas_default_update_callback = 0;

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

// public api
float canvas_limit_mainloop_fps = 240.0;

int canvas(int x, int y, int width, int height, const char *title);
int canvas_window(int x, int y, int width, int height, const char *title);

int canvas_startup();
int canvas_update();
void canvas_color(int window, const float color[4]);
int canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title);

int canvas_close(int window);

int canvas_run(canvas_update_callback update);
int canvas_set_update_callback(int window, canvas_update_callback callback);
int canvas_fixed_update(canvas_time_data *time, double fixed_dt);
void canvas_limit_fps(canvas_time_data *time, double target_fps);
void canvas_sleep(double seconds);

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

bool _canvas_init_platform = false;
bool _canvas_init_gpu = false;
bool _canvas_post_init_ran = false;
bool _canvas_os_timed = false;
bool _canvas_displays_changed = false;

canvas_time_data canvas_main_time = {0};

typedef struct
{
    int index, x, y, width, height, display;
    bool resize, close, titlebar, os_moved, os_resized;
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

canvas_type _canvas[MAX_CANVAS];

canvas_display _canvas_displays[MAX_DISPLAYS];
int _canvas_display_count = 0;
int _canvas_highest_refresh_rate = 60;

#define CANVAS_ERR_BOGUS -69

#ifndef CANVAS_HEADER_ONLY

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

#define INITGUID
#include <windows.h>
#include <dwmapi.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")

static LARGE_INTEGER _canvas_qpc_frequency = {0};
static LARGE_INTEGER _canvas_start_counter = {0};

HINSTANCE _win_instance = NULL;

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

typedef unsigned long _x11_id;
typedef struct _x11_display _x11_display;
typedef _x11_id _x11_window;

typedef struct
{
    int type;
    unsigned long serial;
    int send_event;
    _x11_display *display;
    _x11_window window;
} _x11_event;

extern _x11_display *XOpenDisplay(const char *);
extern _x11_window XCreateSimpleWindow(_x11_display *, _x11_window, int, int, unsigned, unsigned, unsigned, unsigned long, unsigned long);
extern _x11_window XDefaultRootWindow(_x11_display *);
extern unsigned long XBlackPixel(_x11_display *, int);
extern unsigned long XWhitePixel(_x11_display *, int);
extern int XFlush(_x11_display *);
extern int XNextEvent(_x11_display *, _x11_event *);
extern int XCheckMaskEvent(_x11_display *, long, _x11_event *);
extern int XMapWindow(_x11_display *, _x11_window);
extern int XStoreName(_x11_display *, _x11_window, const char *);
extern int XMoveResizeWindow(_x11_display *, _x11_window, int, int, unsigned, unsigned);

extern int XMoveWindow(_x11_display *, _x11_window, int, int);
extern int XResizeWindow(_x11_display *, _x11_window, unsigned, unsigned);

_x11_display *_canvas_display = 0;
bool _canvas_x11_flush = false;
#endif

#ifdef CANVAS_DEBUG
#include <stdio.h>
#define CANVAS_DBG(...) printf(__VA_ARGS__)
#else
#define CANVAS_DBG(...)
#endif

#define CANVAS_BOGUS(window_id)                                \
    if (window_id < 0 || window_id >= MAX_CANVAS)              \
    {                                                          \
        CANVAS_DBG("canvas: bogus window id %d\n", window_id); \
        return CANVAS_ERR_BOGUS;                               \
    }

int _canvas_get_free()
{
    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (_canvas[i].window == NULL)
            return i;
    }
    return -1;
}

int _canvas_window_index(void *window)
{
    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (_canvas[i].window == window)
            return i;
    }
    return -1;
}

//
//
//
#if defined(__APPLE__)

int _canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title)
{
    CANVAS_BOGUS(window_id);

    objc_id window = _canvas[window_id].window;
    if (!window)
        return -1;

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

    return 0;
}

int _canvas_get_window_display(int window_id)
{
    CANVAS_BOGUS(window_id);

    objc_id window = _canvas[window_id].window;
    if (!window)
        return -1;

    objc_id screen = ((MSG_id_id)objc_msgSend)(window, sel_c("screen"));
    if (!screen)
        return 0;

    objc_id device_desc = ((MSG_id_id)objc_msgSend)(screen, sel_c("deviceDescription"));
    if (!device_desc)
        return 0;

    objc_id screen_number_key = ((MSG_id_id_charp)objc_msgSend)(
        cls("NSString"),
        sel_c("stringWithUTF8String:"),
        "NSScreenNumber");

    objc_id display_id_obj = ((MSG_id_id_id)objc_msgSend)(
        device_desc,
        sel_c("objectForKey:"),
        screen_number_key);

    if (!display_id_obj)
        return 0;

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

    return 0;
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
        return -1;
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
}

int _canvas_platform()
{
    _post_init();

    MSG_id_id m = (MSG_id_id)objc_msgSend;
    _mac_app = m(cls("NSApplication"), sel_c("sharedApplication"));
    if (!_mac_app)
        return -1;

    ((MSG_void_id_long)objc_msgSend)(_mac_app, sel_c("setActivationPolicy:"), (long)0);
    ((MSG_void_id_bool)objc_msgSend)(_mac_app, sel_c("activateIgnoringOtherApps:"), 1);

    return 0;
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
        return -1;

    objc_id walloc = m(winClass, sel_c("alloc"));

    typedef objc_id (*MSG_initWin)(objc_id, objc_sel, _CGRect, unsigned long, long, int);
    _CGRect rect = {(double)x, (double)y, (double)width, (double)height};
    objc_id window = ((MSG_initWin)objc_msgSend)(walloc,
                                                 sel_c("initWithContentRect:styleMask:backing:defer:"),
                                                 rect, style, (long)2, 0);

    if (!window)
        return -1;

    ((MSG_void_id_bool)objc_msgSend)(window, sel_c("setTitlebarAppearsTransparent:"), 1);
    ((MSG_void_id_id)objc_msgSend)(window, sel_c("setTitleVisibility:"), (objc_id)1 /*NSWindowTitleHidden*/);

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
    CANVAS_BOGUS(idx);

    _canvas[idx].window = window;
    _canvas[idx].resize = false;
    _canvas[idx].index = idx;

    return idx;
}

int _canvas_gpu_init()
{
    if (_canvas_init_gpu)
        return 0;
    _canvas_init_gpu = 1;

    _mac_device = MTLCreateSystemDefaultDevice();
    if (!_mac_device)
        return -1;

    _metal_queue = ((MSG_id_id)objc_msgSend)(_mac_device, sel_c("newCommandQueue"));
    return _metal_queue ? 0 : -1;
}

int _canvas_update_drawable_size(int window_id)
{
    CANVAS_BOGUS(window_id);

    _CGRect b = ((_CGRect (*)(objc_id, objc_sel))objc_msgSend)(_canvas_data[window_id].view, sel_c("bounds"));
    double w = b.w * _canvas_data[window_id].scale;
    double h = b.h * _canvas_data[window_id].scale;

    struct
    {
        double width;
        double height;
    } sz = {w, h};
    ((void (*)(objc_id, objc_sel, typeof(sz)))objc_msgSend)(_canvas_data[window_id].layer, sel_c("setDrawableSize:"), sz);

    return 0;
}

int _canvas_gpu_new_window(int window_id)
{
    CANVAS_BOGUS(window_id);

    objc_id window = _canvas[window_id].window;

    MSG_id_id m = (MSG_id_id)objc_msgSend;

    objc_id view = m(cls("NSView"), sel_c("alloc"));
    view = ((MSG_id_id_rect)objc_msgSend)(view, sel_c("initWithFrame:"), (_CGRect){0, 0, 800, 600});
    if (!view)
        return -1;

    ((MSG_void_id_bool)objc_msgSend)(view, sel_c("setWantsLayer:"), 1);

    objc_id layer = m(cls("CAMetalLayer"), sel_c("alloc"));
    layer = m(layer, sel_c("init"));
    if (!layer)
        return -1;

    ((MSG_void_id_id)objc_msgSend)(layer, sel_c("setDevice:"), _mac_device);
    ((MSG_void_id_long)objc_msgSend)(layer, sel_c("setPixelFormat:"), 80L /*BGRA8Unorm*/);
    ((MSG_void_id_bool)objc_msgSend)(layer, sel_c("setFramebufferOnly:"), 1);
    ((MSG_void_id_bool)objc_msgSend)(layer, sel_c("setAllowsNextDrawableTimeout:"), 1);

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
        return;

    for (int i = 0; i < MAX_CANVAS; ++i)
    {
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
    return 1;
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
    mach_timebase_info(&_canvas_timebase);
    time->start = mach_absolute_time();
    time->last = 0.0;
    time->frame = 0;
}

double _canvas_get_time(canvas_time_data *time)
{
    uint64_t elapsed = mach_absolute_time() - time->start;
    return (double)elapsed * (double)_canvas_timebase.numer /
           (double)_canvas_timebase.denom / 1e9;
}

int _canvas_close(int window_id)
{
    CANVAS_BOGUS(window_id);

    objc_id window = _canvas[window_id].window;
    if (!window)
        return -1;

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

    return 0;
}

int _canvas_post_update()
{
    return 0;
}

#endif /* __APPLE__ */

//
//
//
#if defined(_WIN32) || defined(_WIN64)

int _canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title)
{
    CANVAS_BOGUS(window_id);

    HWND window = (HWND)_canvas[window_id].window;

    if (!window)
        return -1;

    if (display < 0 || display >= _canvas_display_count)
        display = 0;

    int screen_x = _canvas_displays[display].x + x;
    int screen_y = _canvas_displays[display].y + y;

    SetWindowPos(window, NULL, screen_x, screen_y, width, height, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE);

    if (title)
        SetWindowTextA(window, title);

    _canvas[window_id].display = display;

    return 0;
}

int _canvas_get_window_display(int window_id)
{
    CANVAS_BOGUS(window_id);

    HWND window = (HWND)_canvas[window_id].window;

    if (!window)
        return -1;

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

    return 0;
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

void _canvas_time_init(canvas_time_data *time)
{
    QueryPerformanceFrequency(&_canvas_qpc_frequency);

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    time->start = (uint64_t)counter.QuadPart;
    time->last = 0.0;
    time->frame = 0;
    time->frame_index = 0;
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int window_index = (int)GetWindowLongPtr(hwnd, GWLP_USERDATA);

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
        return 0;
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
        return 0;
    }

    case WM_MOVE:
    {
        // _canvas_update_window_display(window_index);

        return 0;
    }

    case WM_MOVING:
    {
        _canvas[window_index].os_moved = true;

        return 0;
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
        return 0;
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
        return 0;
    }

    case WM_EXITSIZEMOVE:
    {
        if (!_canvas_os_timed)
            break;

        _canvas_os_timed = false;
        KillTimer(hwnd, 1);
        return 0;
    }

    case WM_TIMER:
    {
        if (wParam != 1)
            break;

        canvas_main_loop();

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

    int window_id = -1;
    for (int i = 0; i < MAX_CANVAS; i++)
    {
        if (_canvas[i].window == NULL)
        {
            window_id = i;
            break;
        }
    }

    if (window_id == -1)
        return -1;

    WNDCLASSA wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = _win_instance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "CanvasWindowClass";

    RegisterClassA(&wc);

    DWORD style = WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_VISIBLE;

    _canvas[window_id].index = window_id;

    HWND window = CreateWindowA(
        "CanvasWindowClass",
        title,
        style,
        x, y, width, height,
        NULL, NULL, _win_instance, NULL);

    if (!window)
        return -1;

    _canvas[window_id].window = window;
    _canvas[window_id].resize = false;
    _canvas[window_id].titlebar = false;

    SetWindowLongPtr((HWND)_canvas[window_id].window, GWLP_USERDATA, (LONG_PTR)window_id);

    return window_id;
}

int _canvas_platform()
{
    _win_instance = GetModuleHandle(NULL);

    return 0;
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

    return 1;
}

int _canvas_gpu_init()
{
    if (_canvas_init_gpu)
        return 0;

    _canvas_init_gpu = 1;

    HRESULT result;

    result = CreateDXGIFactory1(
        &IID_IDXGIFactory4,
        (void **)&_win_factory);

    if (FAILED(result))
        return -1;

    result = D3D12CreateDevice(
        NULL,
        D3D_FEATURE_LEVEL_11_0,
        &IID_ID3D12Device,
        (void **)&_win_device);

    if (FAILED(result))
        return -1;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {0};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    result = _win_device->lpVtbl->CreateCommandQueue(
        _win_device,
        &queueDesc,
        &IID_ID3D12CommandQueue,
        (void **)&_win_cmdQueue);

    if (FAILED(result))
        return -1;

    result = _win_device->lpVtbl->CreateCommandAllocator(
        _win_device,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        &IID_ID3D12CommandAllocator,
        (void **)&_win_cmdAllocator);

    if (FAILED(result))
        return -1;

    result = _win_device->lpVtbl->CreateCommandList(
        _win_device,
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        _win_cmdAllocator,
        NULL,
        &IID_ID3D12GraphicsCommandList,
        (void **)&_win_cmdList);

    if (FAILED(result))
        return -1;

    _win_cmdList->lpVtbl->Close(_win_cmdList);

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {0};
    heapDesc.NumDescriptors = 20;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    result = _win_device->lpVtbl->CreateDescriptorHeap(
        _win_device,
        &heapDesc,
        &IID_ID3D12DescriptorHeap,
        (void **)&_win_rtvHeap);

    if (FAILED(result))
        return -1;

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
        return -1;

    _win_fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (!_win_fence_event)
        return -1;

    return 1;
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
    scDesc.Scaling = DXGI_SCALING_STRETCH; // reduces buggy black borders on very fast resize
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
        return -1;

    result = swapChain1->lpVtbl->QueryInterface(
        swapChain1,
        &IID_IDXGISwapChain3,
        (void **)&_canvas_data[window_id].swapChain);
    swapChain1->lpVtbl->Release(swapChain1);

    if (FAILED(result))
        return -1;

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
            return -1;

        _canvas_data[window_id].rtvHandles[i] = rtvHandle;
        _win_device->lpVtbl->CreateRenderTargetView(
            _win_device,
            _canvas_data[window_id].backBuffers[i],
            NULL,
            rtvHandle);
        rtvHandle.ptr += _win_rtvDescriptorSize;
    }

    return 0;
}
int _canvas_window_resize(int window_id)
{
    CANVAS_BOGUS(window_id);

    canvas_data *window = &_canvas_data[window_id];

    if (!window->swapChain || !_canvas[window_id].resize)
        return 0;

    _win_fence_value++;
    _win_cmdQueue->lpVtbl->Signal(_win_cmdQueue, _win_fence, _win_fence_value);
    if (_win_fence->lpVtbl->GetCompletedValue(_win_fence) < _win_fence_value)
    {
        _win_fence->lpVtbl->SetEventOnCompletion(_win_fence, _win_fence_value, _win_fence_event);
        WaitForSingleObject(_win_fence_event, INFINITE);
    }

    _canvas[window_id].resize = false;

    RECT rect;
    GetClientRect((HWND)_canvas[window_id].window, &rect);
    UINT width = rect.right - rect.left;
    UINT height = rect.bottom - rect.top;

    if (width == 0 || height == 0)
        return 0;

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
        return -1;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    _win_rtvHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(_win_rtvHeap, &rtvHandle);
    rtvHandle.ptr += window_id * 2 * _win_rtvDescriptorSize;

    for (int i = 0; i < 2; i++)
    {
        hr = window->swapChain->lpVtbl->GetBuffer(
            window->swapChain, i, &IID_ID3D12Resource,
            (void **)&window->backBuffers[i]);

        if (FAILED(hr))
            return -1;

        window->rtvHandles[i] = rtvHandle;
        _win_device->lpVtbl->CreateRenderTargetView(
            _win_device, window->backBuffers[i], NULL, rtvHandle);
        rtvHandle.ptr += _win_rtvDescriptorSize;
    }

    return 0;
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

    return 0;
}

int _canvas_post_update()
{
    return 0;
}

#endif

//
//
//
#if defined(__linux__)

int _canvas_set(int window_id, int display, int x, int y, int width, int height, const char *title)
{
    if (!_canvas_display)
        return -1;

    CANVAS_BOGUS(window_id);

    _x11_window window = (_x11_window)_canvas[window_id].window;

    if (!window)
        return -1;

    if (title)
        // XStoreName(_canvas_display, window, title); TODO: CRASH! and deadlock

        XMoveResizeWindow(_canvas_display, window, x, y, width, height);
    XFlush(_canvas_display); // TODO: omit this, let the post_update flush handle it (CRASH!)

    return 0;
}

int _canvas_init_displays()
{
    _canvas_display_count = 0;
    _canvas_highest_refresh_rate = 60;

    if (!_canvas_display)
        return -1;

    // TODO: Use Xinerama or XRandR for proper multi-monitor support
    int screen = DefaultScreen(_canvas_display);

    _canvas_displays[0].primary = true;
    _canvas_displays[0].x = 0;
    _canvas_displays[0].y = 0;
    _canvas_displays[0].width = DisplayWidth(_canvas_display, screen);
    _canvas_displays[0].height = DisplayHeight(_canvas_display, screen);
    _canvas_displays[0].refresh_rate = 60;

    _canvas_display_count = 1;

    return _canvas_display_count;
}

int _canvas_get_window_display(int window_id)
{
    CANVAS_BOGUS(window_id);

    return 0;
}

void _canvas_time_init(canvas_time_data *time)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    time->start = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    time->last = 0.0;
    time->frame = 0;
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

int _canvas_window(int x, int y, int width, int height, const char *title)
{
    canvas_startup();

    _x11_window window = 0;

    window = XCreateSimpleWindow(
        _canvas_display,
        XDefaultRootWindow(_canvas_display),
        x, y, width, height, 0,
        XBlackPixel(_canvas_display, 0),
        XWhitePixel(_canvas_display, 0));

    XMapWindow(_canvas_display, window);

    _canvas[idx].window = (canvas_window_handle)window;
    _canvas[idx].resize = false;
    _canvas[idx].index = idx;

    return idx;
}

int _canvas_platform()
{
    _canvas_display = XOpenDisplay(0);

    return 0;
}

int _canvas_update()
{
    _x11_event event;

    while (XCheckMaskEvent(_canvas_display, ~0L, &event))
    {
        XNextEvent(_canvas_display, &event);
    }

    return 1;
}

int _canvas_gpu_init()
{
    if (_canvas_init_gpu)
        return 0;
    _canvas_init_gpu = 1;

    // Select & init backend OpenGL / Vulkan

    return 1;
}

int _canvas_gpu_new_window(int window_id)
{
    CANVAS_BOGUS(window_id);

    return 0;
}

int _canvas_close(int window_id)
{
    CANVAS_BOGUS(window_id);

    _x11_window window = (_x11_window)_canvas[window_id].window;

    if (!window)
        return -1;

    XDestroyWindow(_canvas_display, window);

    return 0;
}

int _canvas_post_update()
{
    XFlush(_canvas_display);

    return 0;
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
        return 0;

    _canvas_init_platform = true;

    _canvas_time_init(&canvas_main_time);

    _canvas_init_displays();

    _canvas_platform();
    return 0;
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

int canvas_run(canvas_update_callback default_callback)
{
    canvas_default_update_callback = default_callback;

    while (1)
    {
        _canvas_os_timed = false;
        canvas_main_loop();
    }

    return 0;
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

    if (x == -1)
        target_x = _canvas_displays[display].width / 2 - width / 2;

    if (y == -1)
        target_y = _canvas_displays[display].height / 2 - height / 2;

    _canvas_set(window_id, display, target_x, target_y, width, height, title);

    return 0;
}

int canvas_window(int x, int y, int width, int height, const char *title)
{
    int window_id = _canvas_window(x, y, width, height, title);

    canvas_set(window_id, 0, x, y, width, height, title);

    if (window_id < 0)
        return -1;

    _canvas_time_init(&_canvas[window_id].time);

    _canvas_get_window_display(window_id);

    return window_id;
}

int canvas(int x, int y, int width, int height, const char *title)
{
    _canvas_gpu_init();

    int window_id = canvas_window(x, y, width, height, title);

    if (window_id < 0)
        return -1;

    canvas_color(window_id, (float[]){0.0f, 0.0f, 0.0f, 1.0f});

    _canvas_gpu_new_window(window_id);

    return window_id;
}

int canvas_close(int window_id)
{
    CANVAS_BOGUS(window_id);

    _canvas_close(window_id);

    _canvas[window_id].window = NULL;

    return 0;
}

int canvas_set_update_callback(int window_id, canvas_update_callback callback)
{
    CANVAS_BOGUS(window_id);

    _canvas[window_id].update = callback;
    return 0;
}

void canvas_color(int window_id, const float color[4])
{
    _canvas[window_id].clear[0] = color[0];
    _canvas[window_id].clear[1] = color[1];
    _canvas[window_id].clear[2] = color[2];
    _canvas[window_id].clear[3] = color[3];
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
