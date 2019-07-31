#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <string>
#include <array>
#include <time.h>

#define WINDOW_WIDTH 821
#define WINDOW_HEIGHT 600

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/examples/imgui_impl_dx9.h"
#include "imgui/examples/imgui_impl_win32.h"

#include "execution_artifacts_output.hpp"

IDirect3D9*           g_d3d9    = nullptr;
IDirect3DDevice9*     g_device  = nullptr;
D3DPRESENT_PARAMETERS g_present = { 0 };

HWND    create_window(WNDCLASSW* wc_out);
bool    create_d3d9_device(HWND hwnd);
void    cleanup_d3d9_device();
void    reset_device();
LRESULT CALLBACK wnd_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

const char* g_usn_warning_message =
    "Due to the size of the Usn Journal, all data from all mounted drives will\n\
be copied to the clipboard and not displayed, do you wish to continue?\n\
Note: It may take up to a minute to extract all data.";

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
        MessageBoxA(0, "Failed to d3d9 device.", "EAT - Error", MB_OK);
        return 0;
    }

    // Show the window
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_device);

    bool   show_demo_window    = true;
    bool   show_another_window = false;
    ImVec4 clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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

        // 1. Show the big demo window (Most of the sample code is in
        // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
        // ImGui!).
        if(show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // Rendering
        ImGui::EndFrame();
        g_device->SetRenderState(D3DRS_ZENABLE, false);
        g_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
        g_device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * 255.0f),
                                              (int)(clear_color.y * 255.0f),
                                              (int)(clear_color.z * 255.0f),
                                              (int)(clear_color.w * 255.0f));
        g_device->Clear(
            0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if(g_device->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_device->EndScene();
        }
        HRESULT result = g_device->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if(result == D3DERR_DEVICELOST &&
           g_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            reset_device();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    cleanup_d3d9_device();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    // create_d3d9_device(wnd);

    ///* GUI */
    // ctx = nk_d3d9_init(device, WINDOW_WIDTH, WINDOW_HEIGHT);

    //{struct nk_font_atlas* atlas;
    // nk_d3d9_font_stash_begin(&atlas);
    // nk_d3d9_font_stash_end();}

    ///* Runtime vars */
    // std::string output_str = "Nothing here yet....";
    // std::string usn_journal_data = "";
    // int         cur_len = output_str.length();
    // bool        interactable = true;
    // bool        prev_interactable = interactable;

    // auto copy_to_clipboard = [&](std::string& data) {
    //	if (OpenClipboard(wnd) && EmptyClipboard()) {
    //		if (HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, data.size())) {
    //			memcpy(GlobalLock(hmem), data.data(), data.size());
    //			SetClipboardData(CF_TEXT, hmem);
    //			GlobalFree(hmem);
    //		}
    //		CloseClipboard();
    //	}
    //};

    // bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
    // while (running)
    //{
    //	/* Input */
    //	MSG msg;
    //	nk_input_begin(ctx);
    //	while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
    //		if (msg.message == WM_QUIT)
    //			running = 0;
    //		TranslateMessage(&msg);
    //		DispatchMessageW(&msg);
    //	}
    //	nk_input_end(ctx);

    //	/* GUI */
    //	if (nk_begin(ctx, "Execution Artifacts Tool", nk_rect(0, 0, WINDOW_WIDTH,
    //WINDOW_HEIGHT), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR))
    //	{
    //		/* Check if the interactable state was changed back on by UsnJournal query. */
    //		if (prev_interactable != interactable && interactable == true) {
    //			copy_to_clipboard(usn_journal_data);
    //			output_str = "Read " + std::to_string(usn_journal_data.size() / 1024 / 1024) +
    //"MB from UsnJournal.."; 			usn_journal_data = "";
    //		}
    //		prev_interactable = interactable;

    //		/* Button wrapper to enable non-immediate operations with the interactable
    //boolean. */ 		auto button = [&interactable, ctx](const char* name) { 			return
    //nk_button_label(ctx, name) && interactable;
    //		};

    //		nk_layout_row_dynamic(ctx, 25, 6);
    //		if (button("Usn Journal")) {
    //			if (MessageBoxA(wnd, g_usn_warning_message, "Warning", MB_ICONWARNING |
    //MB_OKCANCEL) == 1) { 				interactable = false;
    //				ea::get_usn_journal_info_deferred(usn_journal_data, interactable);
    //				output_str = "Querying UsnJournal....";
    //			}
    //		}

    //		if (button("UserAssist")) {
    //			output_str = ea::get_user_assist_info();
    //		}

    //		if (button("AppCompatFlags")) {
    //			output_str = ea::get_app_compat_flags_info();
    //		}

    //		if (button("MUI Cache")) {
    //			output_str = ea::get_mui_cache_info();
    //		}

    //		if (button("RecentApps")) {
    //			output_str = ea::get_recent_apps_info();
    //		}

    //		if (button("ShimCache")) {
    //			output_str = ea::get_shim_cache_info();
    //		}

    //		cur_len = output_str.length();;

    //		nk_layout_row_dynamic(ctx, 30, 1);
    //		nk_label(ctx, "Output:", NK_TEXT_CENTERED);

    //		nk_layout_row_dynamic(ctx, 492, 1);
    //		nk_edit_string(ctx, NK_EDIT_SELECTABLE | NK_EDIT_MULTILINE | NK_EDIT_ALLOW_TAB |
    //NK_EDIT_CLIPBOARD, 			output_str.data(), &cur_len, output_str.size(),
    //nk_filter_default);

    //		nk_layout_row_static(ctx, 25, 160, 1);
    //		if (nk_button_label(ctx, "Copy to Clipboard") && interactable) {
    //			copy_to_clipboard(output_str);
    //		}
    //	}
    //	nk_end(ctx);

    //	/* Draw */
    //	{
    //		HRESULT hr;
    //		hr = IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER
    //| D3DCLEAR_STENCIL, 			D3DCOLOR_COLORVALUE(bg.r, bg.g, bg.b, bg.a), 0.0f, 0);

    //		hr = IDirect3DDevice9_BeginScene(device);
    //		nk_d3d9_render(NK_ANTI_ALIASING_ON);
    //		hr = IDirect3DDevice9_EndScene(device);

    //		if (deviceEx) {
    //			hr = IDirect3DDevice9Ex_PresentEx(deviceEx, NULL, NULL, NULL, NULL, 0);
    //		}
    //		else {
    //			hr = IDirect3DDevice9_Present(device, NULL, NULL, NULL, NULL);
    //		}
    //		if (hr == D3DERR_DEVICELOST || hr == D3DERR_DEVICEHUNG || hr ==
    //D3DERR_DEVICEREMOVED) {
    //			/* to recover from this, you'll need to recreate device and all the resources
    //*/ 			MessageBoxW(NULL, L"D3D9 device is lost or removed!", L"Error", 0); 			break;
    //		}
    //		else if (hr == S_PRESENT_OCCLUDED) {
    //			/* window is not visible, so vsync won't work. Let's sleep a bit to reduce CPU
    //usage */ 			Sleep(10);
    //		}
    //	}
    //}
    // nk_d3d9_shutdown();
    // if (deviceEx)IDirect3DDevice9Ex_Release(deviceEx);
    // else IDirect3DDevice9_Release(device);
    // UnregisterClassW(wc.lpszClassName, wc.hInstance);
    // if (!interactable)
    //	exit(0); /* This is not ideal but gets the job done. */
    return 0;
}

HWND create_window(WNDCLASSW* wc_out)
{
    auto& wc      = *wc_out;
    RECT  rect    = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    DWORD style   = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    DWORD exstyle = WS_EX_APPWINDOW;
    HWND  wnd;
    int   running = 1;

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