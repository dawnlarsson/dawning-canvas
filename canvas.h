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

// call first to init the native platform dependencies
int canvas_startup();

int canvas_window(int width, int height, const char *title);
int canvas_new(int width, int height, const char *title);

bool __canvas_startup_ran = false;
bool __post_init_ran = false;

#ifdef CANVAS_IMPL

int __canvas_platform();
int __canvas_update();

int canvas_startup()
{
    if (__canvas_startup_ran)
        return 0;

    __canvas_startup_ran = true;

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

typedef void *objc_id;
typedef void *objc_sel;
typedef struct
{
    double x, y, w, h;
} CGRect;
objc_id objc_msgSend(objc_id, objc_sel, ...);
objc_id objc_getClass(const char *);
objc_sel sel_registerName(const char *);
typedef objc_id (*InitRect)(objc_id, objc_sel, CGRect, long, long, int);

objc_id __mac_pool;
objc_id __mac_app;

void __post_init()
{
    if (__post_init_ran)
        return;

    __post_init_ran = true;

    objc_msgSend(__mac_app, sel_registerName("run"));
    objc_msgSend(__mac_pool, sel_registerName("drain"));
}

objc_id __canvas_window(int x, int y, int width, int height, const char *title)
{
    canvas_startup();

    objc_id window = ((InitRect)objc_msgSend)(
        objc_msgSend(objc_getClass("NSWindow"), sel_registerName("alloc")),
        sel_registerName("initWithContentRect:styleMask:backing:defer:"),
        (CGRect){x, y, width, height}, 15, 2, 0);

    // Enable fullscreen support
    objc_msgSend(window, sel_registerName("setCollectionBehavior:"), 128);

    objc_msgSend(window, sel_registerName("setTitlebarAppearsTransparent:"), 1);
    objc_msgSend(window, sel_registerName("setMovableByWindowBackground:"), 1);
    objc_msgSend(window, sel_registerName("makeKeyAndOrderFront:"), 1);

    return window;
}

int __canvas_platform()
{
    __mac_pool = objc_msgSend(objc_msgSend(objc_getClass("NSAutoreleasePool"), sel_registerName("alloc")), sel_registerName("init"));
    __mac_app = objc_msgSend(objc_getClass("NSApplication"), sel_registerName("sharedApplication"));
    objc_msgSend(__mac_app, sel_registerName("setActivationPolicy:"), 0);

    objc_msgSend(__mac_app, sel_registerName("activateIgnoringOtherApps:"), 1);

    return 0;
}

int __canvas_update()
{
    __post_init();

    return 1;
}

#endif // MacOS

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
#endif

#endif // CANVAS_IMPL