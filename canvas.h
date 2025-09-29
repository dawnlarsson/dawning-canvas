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
        Windows                 \       ~       DirectX12   -lgdi32 -luser32 -mwindows -ldwmapi
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

        clang example/simple.c -framework Cocoa -I. -Oz -fno-ident -fno-unwind-tables -fno-asynchronous-unwind-tables -fomit-frame-pointer -fno-stack-protector -Wl,-dead_strip && ./a.out

        -  Info  -
        Canvas is created by Dawn Larsson 2025
        This is licensed under Apache License 2.0, by Dawn Larsson

        https://dawning.dev/  -  https://docs.dawning.dev/

        repo
        https://github.com/dawnlarsson/dawning-canvas

*/

#pragma once

#define MAX_CANVAS 32

#include <stdbool.h>

int canvas_window(int width, int height, const char *title);
int canvas_new(int width, int height, const char *title);
int canvas_color(int window, const float color[4]);
int canvas_startup();
int canvas_update();

bool _canvas_init_platform = false;
bool _canvas_init_gpu = false;
bool _post_init_ran = false;

int _canvas_platform();
int _canvas_update();

typedef void *canvas_window_handle;

typedef struct
{
    canvas_window_handle window;
    float clear[4];

} canvas_type;

static int _canvas_count = 0;

#ifndef CANVAS_HEADER_ONLY

#if defined(__APPLE__)

#include <TargetConditionals.h>
#include <objc/objc.h>
#include <objc/message.h>
typedef void *objc_id;
typedef void *objc_sel;
static objc_id _mac_pool = NULL;
static objc_id _mac_app = NULL;
static objc_id _mac_device = NULL;
static objc_id _metal_queue = NULL;


typedef struct
{
    objc_id window;
    objc_id view;
    objc_id layer;
    double clear[4]; /* rgba */
    double scale;
} _mac_canvas;

typedef struct
{
    double r, g, b, a;
} _MTLClearColor;

typedef struct
{
    double x, y, w, h;
} _CGRect;

static _mac_canvas _canvas[MAX_CANVAS];

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

ID3D12Device *g_device = NULL;
ID3D12CommandQueue *g_cmdQueue = NULL;
IDXGIFactory4 *g_factory = NULL;
ID3D12CommandAllocator *g_cmdAllocator = NULL;
ID3D12GraphicsCommandList *g_cmdList = NULL;
ID3D12DescriptorHeap *g_rtvHeap = NULL;
ID3D12Fence *g_fence = NULL;
UINT64 g_fenceValue = 0;
HANDLE g_fenceEvent = NULL;
UINT g_rtvDescriptorSize = 0;

typedef struct
{
    HWND hwnd;
    IDXGISwapChain3 *swapChain;
    ID3D12Resource *backBuffers[2];
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
    float clear[4];
    BOOL needsResize;
    int windowIndex;
} WindowContext;

typedef struct
{
    BOOL titlebarless;
    WindowContext *ctx;
} WindowData;

HINSTANCE __win_instance = NULL;
WindowContext _canvas[MAX_CANVAS];
#endif

int canvas_startup()
{
    if (_canvas_init_platform)
        return 0;

    _canvas_init_platform = true;

    _canvas_platform();
    return 0;
}

int canvas_update()
{
    return _canvas_update();
}

//
//
//
#if defined(__APPLE__)

static void _post_init()
{
    if (_post_init_ran)
        return;
    _post_init_ran = 1;
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

canvas_window_handle _canvas_window(int x, int y, int width, int height, const char *title)
{
    _post_init();
    canvas_startup();

    unsigned long style =
        (1UL << 0) /*Titled*/ | (1UL << 1) /*Closable*/ |
        (1UL << 2) /*Mini*/ | (1UL << 3) /*Resizable*/;

    MSG_id_id m = (MSG_id_id)objc_msgSend;

    objc_id winClass = cls("NSWindow");
    if (!winClass)
        return NULL;
    objc_id walloc = m(winClass, sel_c("alloc"));

    typedef objc_id (*MSG_initWin)(objc_id, objc_sel, _CGRect, unsigned long, long, int);
    _CGRect rect = {(double)x, (double)y, (double)width, (double)height};
    objc_id window = ((MSG_initWin)objc_msgSend)(walloc,
                                                 sel_c("initWithContentRect:styleMask:backing:defer:"),
                                                 rect, style, (long)2, 0);
    if (!window)
        return NULL;

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
    return window;
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

static void _canvas_update_drawable_size(_mac_canvas *c)
{
    if (!c || !c->view || !c->layer)
        return;
    _CGRect b = ((_CGRect (*)(objc_id, objc_sel))objc_msgSend)(c->view, sel_c("bounds"));
    double w = b.w * c->scale;
    double h = b.h * c->scale;

    struct
    {
        double width;
        double height;
    } sz = {w, h};
    ((void (*)(objc_id, objc_sel, typeof(sz)))objc_msgSend)(c->layer, sel_c("setDrawableSize:"), sz);
}

static int _canvas_gpu_new_window(objc_id window)
{
    if (!window || !_mac_device || _canvas_count >= MAX_CANVAS)
        return -1;

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

    int idx = _canvas_count++;
    _canvas[idx].window = window;
    _canvas[idx].view = view;
    _canvas[idx].layer = layer;
    _canvas[idx].scale = scale;
    _canvas[idx].clear[0] = 0.0;
    _canvas[idx].clear[1] = 0.0;
    _canvas[idx].clear[2] = 0.0;
    _canvas[idx].clear[3] = 1.0;

    return idx;
}

static void _canvas_gpu_draw_all(void)
{
    if (!_metal_queue)
        return;

    for (int i = 0; i < _canvas_count; ++i)
    {
        _canvas_update_drawable_size(&_canvas[i]);

        objc_id layer = _canvas[i].layer;
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
        ((MSG_void_id_id)objc_msgSend)(_mac_app, sel_c("sendEvent:"), ev);
    }

    ((MSG_void_id)objc_msgSend)(_mac_app, sel_c("updateWindows"));

    _canvas_gpu_draw_all();

    if (framePool)
        ((MSG_void_id)objc_msgSend)(framePool, sel_c("drain"));
    return 1;
}

#endif /* __APPLE__ */

//
//
//
#if defined(__linux__)

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
extern int XMapWindow(_x11_display *, _x11_window);

_x11_display *_canvas_display = 0;

_x11_window _canvas_window(int x, int y, int width, int height, const char *title)
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
    XFlush(_canvas_display);

    return window;
}

int _canvas_platform()
{
    _canvas_display = XOpenDisplay(0);

    return 0;
}

int _canvas_update()
{
    _x11_event event;
    while (XNextEvent(_canvas_display, &event) == 0)
    {
    }

    return 1;
}
#endif // _linux_

//
//
//
#if defined(_WIN32) || defined(_WIN64)

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

canvas_window_handle _canvas_window(int x, int y, int width, int height, const char *title)
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

        
    int idx = _canvas_count++;
    _canvas[idx].hwnd = window;
    _canvas[idx].needsResize = FALSE;
    _canvas[idx].windowIndex = idx;
    _canvas[idx].clear[0] = 0.0f;
    _canvas[idx].clear[1] = 0.0f;
    _canvas[idx].clear[2] = 0.0f;
    _canvas[idx].clear[3] = 1.0f;

    return window;
}

int _canvas_platform()
{
    __win_instance = GetModuleHandle(NULL);

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

    return 1;
}

int _canvas_gpu_init()
{
    if (_canvas_init_gpu)
        return 0;
    _canvas_init_gpu = 1;
    // Initialize DirectX 12



    return 1;
}

int _canvas_gpu_new_window(HWND hwnd)
{
    if (!hwnd || _canvas_count >= MAX_CANVAS)
        return -1;
}

#endif


int canvas(int x, int y, int width, int height, const char *title)
{
    _canvas_gpu_init();
    canvas_window_handle window = _canvas_window(x, y, width, height, title);
    int id = -1;

    id = _canvas_gpu_new_window(window);

    return id;
}

int canvas_color(int window, const float color[4])
{
    _canvas[window].clear[0] = color[0];
    _canvas[window].clear[1] = color[1];
    _canvas[window].clear[2] = color[2];
    _canvas[window].clear[3] = color[3];
}

#endif // CANVAS_HEADER_ONLY
