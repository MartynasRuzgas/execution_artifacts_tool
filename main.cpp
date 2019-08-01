/*
Copyright (c) 2019 Martynas Ruzgas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <string>
#include <array>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/examples/imgui_impl_dx9.h"
#include "imgui/examples/imgui_impl_win32.h"

#include "eat/eat.hpp"

IDirect3D9*           g_d3d9    = nullptr;
IDirect3DDevice9*     g_device  = nullptr;
D3DPRESENT_PARAMETERS g_present = { 0 };

HWND    create_window(WNDCLASSW* wc_out);
bool    create_d3d9_device(HWND hwnd);
void    cleanup_d3d9_device();
void    reset_device();
LRESULT CALLBACK wnd_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow)
{
    WNDCLASSW wc;
    auto      hwnd = create_window(&wc);
    if(!hwnd) {
        MessageBoxA(0, "Failed to create window.", "EAT - Error", MB_OK);
        return 0;
    }

    if(!create_d3d9_device(hwnd)) {
        MessageBoxA(0, "Failed to d3d9 create device.", "EAT - Error", MB_OK);
        return 0;
    }

    // Show the window
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_device);

    // Setup our own style
    eat::on_initialize();

    MSG msg = { 0 };
    while(msg.message != WM_QUIT) {
        if(PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Do our UI
        eat::on_frame(hwnd);

        // Rendering
        ImGui::EndFrame();
        g_device->SetRenderState(D3DRS_ZENABLE, false);
        g_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
        g_device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(255, 255, 255, 255);
        g_device->Clear(
            0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if(g_device->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_device->EndScene();
        }

        // Handle loss of D3D9 device
        if(g_device->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST &&
           g_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            reset_device();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    cleanup_d3d9_device();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

HWND create_window(WNDCLASSW* wc_out)
{
    auto& wc      = *wc_out;
    RECT  rect    = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    DWORD style   = WS_OVERLAPPEDWINDOW;
    DWORD exstyle = WS_EX_APPWINDOW;

    memset(&wc, 0, sizeof(wc));
    wc.style         = CS_DBLCLKS;
    wc.lpfnWndProc   = wnd_proc;
    wc.hInstance     = GetModuleHandleW(0);
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"eat";
    RegisterClassW(&wc);

    AdjustWindowRectEx(&rect, style, FALSE, exstyle);

    return CreateWindowExW(exstyle,
                           wc.lpszClassName,
                           L"Execution Artifacts Tool",
                           style | WS_VISIBLE,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           rect.right - rect.left,
                           rect.bottom - rect.top,
                           NULL,
                           NULL,
                           wc.hInstance,
                           NULL);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND   hWnd,
                                              UINT   msg,
                                              WPARAM wParam,
                                              LPARAM lParam);
LRESULT CALLBACK wnd_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if(ImGui_ImplWin32_WndProcHandler(wnd, msg, wparam, lparam))
        return true;

    switch(msg) {
    case WM_SIZE:
        if(g_device != NULL && wparam != SIZE_MINIMIZED) {
            g_present.BackBufferWidth  = LOWORD(lparam);
            g_present.BackBufferHeight = HIWORD(lparam);
            reset_device();
        }
        return 0;
    case WM_SYSCOMMAND:
        if((wparam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(wnd, msg, wparam, lparam);
}

bool create_d3d9_device(HWND hwnd)
{
    if((g_d3d9 = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    g_present.Windowed               = TRUE;
    g_present.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    g_present.BackBufferFormat       = D3DFMT_UNKNOWN;
    g_present.EnableAutoDepthStencil = TRUE;
    g_present.AutoDepthStencilFormat = D3DFMT_D16;
    g_present.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;

    // Attempt to create the device.
    bool                 success         = false;
    std::array<DWORD, 2> behaviour_flags = { D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                             D3DCREATE_SOFTWARE_VERTEXPROCESSING };
    for(auto behaviour_flag : behaviour_flags) {
        if(g_d3d9->CreateDevice(D3DADAPTER_DEFAULT,
                                D3DDEVTYPE_HAL,
                                hwnd,
                                behaviour_flag,
                                &g_present,
                                &g_device) >= 0) {
            success = true;
            break;
        }
    }

    return success;
}

void cleanup_d3d9_device()
{
    if(g_device) {
        g_device->Release();
        g_device = nullptr;
    }
    if(g_d3d9) {
        g_d3d9->Release();
        g_d3d9 = nullptr;
    }
}

void reset_device()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    if(g_device->Reset(&g_present) == D3DERR_INVALIDCALL) {
        MessageBoxA(0, "Failed to reset the D3D device.", "EAT - Error", MB_OK);
        exit(0);
    }
    ImGui_ImplDX9_CreateDeviceObjects();
}