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
        Windows                 ~       ~       DirectX12   -lgdi32 -luser32 -mwindows -ldwmapi
        MacOS                   ~       ~       Metal       -framework Cocoa
        Linux                   ~       ~       Vulkan      -lX11
        iOS                     ~       ~       Metal
        Android                 ~       ~       Vulkan
        HTML5                   ~       ~       WebGPU

        * note for windows:
        x86_64-w64-mingw32-gcc for cross compiling to windows

        -  Building  -

        -  Info  -
        Canvas is created by Dawn Larsson 2025
        This is licensed under Apache License 2.0, by Dawn Larsson

        https://dawning.dev/  -  https://docs.dawning.dev/

        repo
        https://github.com/dawnlarsson/dawning-canvas

*/

#pragma once

#define false 0
#define true 1

#define bool _Bool
#if __STDC_VERSION__ < 199901L && __GNUC__ < 3
typedef int _Bool;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

// call first to init the native platform dependencies
int canvas_startup();

int canvas_window(int width, int height, const char *title);
int canvas_new(int width, int height, const char *title);

bool __canvas_init_platform = false;
bool __canvas_init_gpu = false;
bool __post_init_ran = false;

#ifdef CANVAS_IMPL

int __canvas_platform();
int __canvas_update();

int canvas_startup()
{
    if (__canvas_init_platform)
        return 0;

    __canvas_init_platform = true;

    __canvas_platform();
    return 0;
}

int canvas_update()
{
    return __canvas_update();
}

//
//
//
#if defined(__APPLE__)

#include <TargetConditionals.h>
#include <objc/objc.h>
#include <objc/message.h>

typedef void *objc_id;
typedef void *objc_sel;

static objc_id __mac_pool = NULL;
static objc_id __mac_app = NULL;
static objc_id __mac_device = NULL;

#define EVENT_MASK_ANY ((unsigned long long)-1)

typedef struct
{
    double x, y, w, h;
} CGRect;

typedef objc_id (*MSG_id_id_sel_noargs)(objc_id, objc_sel);
typedef objc_id (*MSG_id_id_sel_charp)(objc_id, objc_sel, const char *);
typedef objc_id (*MSG_id_id_sel_rect_ul_long_int)(objc_id, objc_sel, CGRect, unsigned long, long, int);
typedef objc_id (*MSG_id_id_sel_id_sel_bool)(objc_id, objc_sel, objc_id, int);
typedef objc_id (*MSG_id_id_sel_rect_ulong_long_int)(objc_id, objc_sel, CGRect, unsigned long long, long, int);
typedef objc_id (*MSG_id_id_sel_noargs_return_id)(objc_id, objc_sel);
typedef void (*MSG_void_id_sel_id)(objc_id, objc_sel, objc_id);
typedef void (*MSG_void_id_sel_long)(objc_id, objc_sel, long);
typedef void (*MSG_void_id_sel_bool)(objc_id, objc_sel, int);

typedef objc_id (*MSG_nextEventFn)(objc_id, objc_sel,
                                   unsigned long long, /* mask (NSEventMask) */
                                   objc_id,            /* untilDate (NSDate*) */
                                   objc_id,            /* inMode (NSString*) */
                                   signed char);       /* dequeue (BOOL) */

static inline objc_sel sel_c(const char *s) { return sel_registerName(s); }
static inline objc_id cls(const char *s) { return objc_getClass(s); }

void __post_init()
{
    if (__post_init_ran)
        return;
    __post_init_ran = 1;

    objc_id poolClass = cls("NSAutoreleasePool");
    if (!poolClass)
        return;

    MSG_id_id_sel_noargs poolAlloc = (MSG_id_id_sel_noargs)objc_msgSend;
    objc_id alloced = poolAlloc(poolClass, sel_c("alloc"));

    MSG_id_id_sel_noargs poolInit = (MSG_id_id_sel_noargs)objc_msgSend;
    __mac_pool = poolInit(alloced, sel_c("init"));
}

objc_id __canvas_window(int x, int y, int width, int height, const char *title)
{
    __post_init();
    canvas_startup();

    unsigned long style =
        (1UL << 0)   /* NSWindowStyleMaskTitled */
        | (1UL << 1) /* NSWindowStyleMaskClosable */
        | (1UL << 2) /* NSWindowStyleMaskMiniaturizable */
        | (1UL << 3) /* NSWindowStyleMaskResizable */
        ;

    objc_id nsWindowClass = cls("NSWindow");

    if (!nsWindowClass)
        return NULL;

    MSG_id_id_sel_noargs alloc_fn = (MSG_id_id_sel_noargs)objc_msgSend;
    objc_id windowAlloc = alloc_fn(nsWindowClass, sel_c("alloc"));

    MSG_id_id_sel_rect_ul_long_int initWithRect = (MSG_id_id_sel_rect_ul_long_int)objc_msgSend;
    CGRect rect = {(double)x, (double)y, (double)width, (double)height};
    objc_id window = initWithRect(windowAlloc,
                                  sel_c("initWithContentRect:styleMask:backing:defer:"),
                                  rect,
                                  (unsigned long)style,
                                  (long)2, /* NSBackingStoreBuffered */
                                  (int)0 /* defer = NO */);

    if (!window)
    {
        return NULL;
    }

    MSG_void_id_sel_long setCollectionBehavior = (MSG_void_id_sel_long)objc_msgSend;
    setCollectionBehavior(window, sel_c("setCollectionBehavior:"), (long)128);

    MSG_void_id_sel_bool setBool = (MSG_void_id_sel_bool)objc_msgSend;
    setBool(window, sel_c("setTitleVisibility:"), 1); /* NSWindowTitleHidden */
    setBool(window, sel_c("setTitlebarAppearsTransparent:"), 1);
    setBool(window, sel_c("setMovableByWindowBackground:"), 1);

    MSG_void_id_sel_id makeKey = (MSG_void_id_sel_id)objc_msgSend;
    makeKey(window, sel_c("makeKeyAndOrderFront:"), (objc_id)0);

    if (title)
    {
        objc_id nsStringClass = cls("NSString");
        MSG_id_id_sel_charp stringWithUTF8 = (MSG_id_id_sel_charp)objc_msgSend;
        objc_id nsTitle = stringWithUTF8(nsStringClass, sel_c("stringWithUTF8String:"), title);

        if (nsTitle)
            makeKey(window, sel_c("setTitle:"), nsTitle);
    }

    return window;
}

int __canvas_platform()
{
    __post_init();

    objc_id appClass = cls("NSApplication");
    if (!appClass)
        return -1;

    MSG_id_id_sel_noargs sharedApp = (MSG_id_id_sel_noargs)objc_msgSend;
    __mac_app = sharedApp(appClass, sel_c("sharedApplication"));

    if (!__mac_app)
        return -1;

    MSG_void_id_sel_long setActivationPolicy = (MSG_void_id_sel_long)objc_msgSend;
    setActivationPolicy(__mac_app, sel_c("setActivationPolicy:"), (long)0);

    MSG_void_id_sel_bool activateIgnoring = (MSG_void_id_sel_bool)objc_msgSend;
    activateIgnoring(__mac_app, sel_c("activateIgnoringOtherApps:"), 1);

    return 0;
}

int __canvas_gpu_init()
{
    if (__canvas_init_gpu)
        return 0;

    __canvas_init_gpu = true;

    objc_id mtlDeviceClass = cls("MTLDevice");
    if (!mtlDeviceClass)
        return -1;

    MSG_id_id_sel_noargs getSys = (MSG_id_id_sel_noargs)objc_msgSend;
    __mac_device = getSys(mtlDeviceClass, sel_c("systemDefaultDevice"));

    if (!__mac_device)
        return -1;

    return 0;
}

int __canvas_gpu_new_window(objc_id window)
{
    if (!window || !__mac_device)
        return -1;

    objc_id nsViewClass = cls("NSView");
    if (!nsViewClass)
        return -1;

    MSG_id_id_sel_noargs alloc_fn = (MSG_id_id_sel_noargs)objc_msgSend;
    objc_id viewAlloc = alloc_fn(nsViewClass, sel_c("alloc"));

    MSG_id_id_sel_rect_ulong_long_int initWithFrame = (MSG_id_id_sel_rect_ulong_long_int)objc_msgSend;
    CGRect rect = {0, 0, 800, 600};
    objc_id view = initWithFrame(viewAlloc,
                                 sel_c("initWithFrame:"),
                                 rect,
                                 (unsigned long long)0,
                                 (long)0,
                                 (int)0);

    if (!view)
        return -1;

    MSG_void_id_sel_id setDevice = (MSG_void_id_sel_id)objc_msgSend;
    setDevice(view, sel_c("setDevice:"), __mac_device);

    MSG_void_id_sel_id setContentView = (MSG_void_id_sel_id)objc_msgSend;
    setContentView(window, sel_c("setContentView:"), view);

    return 0;
}

int __canvas_update()
{
    __post_init();

    objc_id poolClass = cls("NSAutoreleasePool");
    objc_id framePool = NULL;
    if (poolClass)
    {
        MSG_id_id_sel_noargs poolAlloc = (MSG_id_id_sel_noargs)objc_msgSend;
        objc_id tmp = poolAlloc(poolClass, sel_c("alloc"));
        MSG_id_id_sel_noargs poolInit = (MSG_id_id_sel_noargs)objc_msgSend;
        framePool = poolInit(tmp, sel_c("init"));
    }

    objc_id ns_mode = NULL;
    objc_id distantPast = NULL;

    objc_id nsStringClass = cls("NSString");
    if (nsStringClass)
    {
        MSG_id_id_sel_charp stringWithUTF8 = (MSG_id_id_sel_charp)objc_msgSend;
        ns_mode = stringWithUTF8(nsStringClass, sel_c("stringWithUTF8String:"), "kCFRunLoopDefaultMode");
    }

    objc_id nsDateClass = cls("NSDate");
    if (nsDateClass)
    {
        MSG_id_id_sel_noargs getDistant = (MSG_id_id_sel_noargs)objc_msgSend;
        distantPast = getDistant(nsDateClass, sel_c("distantPast"));
    }

    MSG_nextEventFn nextEvent = (MSG_nextEventFn)objc_msgSend;

    for (;;)
    {
        objc_id event = nextEvent(
            __mac_app,
            sel_c("nextEventMatchingMask:untilDate:inMode:dequeue:"),
            EVENT_MASK_ANY,
            distantPast,
            ns_mode,
            (signed char)1);

        if (!event)
            break;

        MSG_void_id_sel_id sendEvent = (MSG_void_id_sel_id)objc_msgSend;
        sendEvent(__mac_app, sel_c("sendEvent:"), event);
    }

    typedef void (*MSG_void_id_sel_noargs)(objc_id, objc_sel);
    MSG_void_id_sel_noargs updateWindows = (MSG_void_id_sel_noargs)objc_msgSend;
    updateWindows(__mac_app, sel_c("updateWindows"));

    if (framePool)
    {
        MSG_id_id_sel_noargs drain = (MSG_id_id_sel_noargs)objc_msgSend;
        drain(framePool, sel_c("drain"));
    }

    return 1;
}

#endif /* __APPLE__ */

//
//
//
#if defined(_WIN32) || defined(_WIN64)

#define INITGUID
#include <windows.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

HINSTANCE __win_instance = NULL;

typedef struct
{
    HWND hwnd;
    // IDXGISwapChain3 *swapChain;
    // ID3D12Resource *backBuffers[2];
    // D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
    float clearColor[4]; // Per-window clear color
    BOOL needsResize;
    int windowIndex;
} WindowContext;

typedef struct
{
    BOOL titlebarless;
    WindowContext *ctx;
} WindowData;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    WindowData *pData = (WindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg)
    {
    case WM_CREATE:
    {
        CREATESTRUCT *pCreate = (CREATESTRUCT *)lParam;
        WindowData *pNewData = (WindowData *)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pNewData);

        if (pNewData && pNewData->titlebarless)
        {
            enum DWMNCRENDERINGPOLICY policy = DWMNCRP_ENABLED;
            DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &policy, sizeof(policy));

            DWM_WINDOW_CORNER_PREFERENCE corner = DWMWCP_ROUND;
            DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));
        }
        return 0;
    }

    case WM_NCCALCSIZE:
    {
        if (wParam == TRUE && pData && pData->titlebarless)
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
        if (pData && pData->titlebarless)
        {
            LRESULT hit = DefWindowProc(hwnd, msg, wParam, lParam);

            if (hit == HTCAPTION)
                return HTCLIENT;

            if (hit == HTCLIENT)
            {
                POINT pt = {LOWORD(lParam), HIWORD(lParam)};
                ScreenToClient(hwnd, &pt);

                RECT rcWindow;
                GetClientRect(hwnd, &rcWindow);

                if (pt.y < 30 && pt.y >= 0)
                {
                    if (pt.x < rcWindow.right - 140)
                    {
                        return HTCAPTION;
                    }
                }
            }

            return hit;
        }
        break;
    }

    case WM_SIZE:
    {
        if (pData && pData->ctx && wParam != SIZE_MINIMIZED)
        {
            pData->ctx->needsResize = TRUE;
        }
        return 0;
    }

    case WM_DESTROY:
        if (pData)
            free(pData);

        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND __canvas_window(int x, int y, int width, int height, const char *title)
{
    bool titlebarless = true;

    canvas_startup();

    WNDCLASSA wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = __win_instance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "CanvasWindowClass";

    RegisterClassA(&wc);

    WindowContext *ctx = (WindowContext *)malloc(sizeof(WindowContext));

    WindowData *pData = (WindowData *)malloc(sizeof(WindowData));
    pData->titlebarless = titlebarless;
    pData->ctx = ctx;

    DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

    if (titlebarless)
        style = WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_VISIBLE;

    HWND window = CreateWindowA(
        "CanvasWindowClass",
        title,
        style,
        x, y, width, height,
        NULL, NULL, __win_instance, pData);

    if (window && titlebarless)
        SetWindowPos(window, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

    return window;
}

int __canvas_platform()
{
    __win_instance = GetModuleHandle(NULL);

    return 0;
}

int __canvas_update()
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 1;
}

#endif

//
//
//
#if defined(__linux__)

typedef unsigned long __x11_id;
typedef struct __x11_display __x11_display;
typedef __x11_id __x11_window;

typedef struct
{
    int type;
    unsigned long serial;
    int send_event;
    __x11_display *display;
    __x11_window window;
} __x11_event;

extern __x11_display *XOpenDisplay(const char *);
extern __x11_window XCreateSimpleWindow(__x11_display *, __x11_window, int, int, unsigned, unsigned, unsigned, unsigned long, unsigned long);
extern __x11_window XDefaultRootWindow(__x11_display *);
extern unsigned long XBlackPixel(__x11_display *, int);
extern unsigned long XWhitePixel(__x11_display *, int);
extern int XFlush(__x11_display *);
extern int XNextEvent(__x11_display *, __x11_event *);
extern int XMapWindow(__x11_display *, __x11_window);

__x11_display *__canvas_display = 0;

__x11_window __canvas_window(int x, int y, int width, int height, const char *title)
{
    canvas_startup();

    __x11_window window = 0;

    window = XCreateSimpleWindow(
        __canvas_display,
        XDefaultRootWindow(__canvas_display),
        x, y, width, height, 0,
        XBlackPixel(__canvas_display, 0),
        XWhitePixel(__canvas_display, 0));

    XMapWindow(__canvas_display, window);
    XFlush(__canvas_display);

    return window;
}

int __canvas_platform()
{
    __canvas_display = XOpenDisplay(0);

    return 0;
}

int __canvas_update()
{
    __x11_event event;
    while (XNextEvent(__canvas_display, &event) == 0)
    {
    }

    return 1;
}
#endif // __linux__

int canvas(int x, int y, int width, int height, const char *title)
{
    __canvas_gpu_init();

    void *window = (void *)__canvas_window(x, y, width, height, title);

    __canvas_gpu_new_window(window);
}

#endif // CANVAS_IMPL
