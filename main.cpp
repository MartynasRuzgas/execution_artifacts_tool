#define COBJMACROS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <string>
#include <time.h>

#define WINDOW_WIDTH 821
#define WINDOW_HEIGHT 600

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_D3D9_IMPLEMENTATION

#include <nuklear.h>
#include <demo/d3d9/nuklear_d3d9.h>

#include "execution_artifacts_output.hpp"

IDirect3DDevice9* device;
IDirect3DDevice9Ex* deviceEx;
D3DPRESENT_PARAMETERS present;

const char* g_usn_warning_message =
"Due to the size of the Usn Journal, all data from all mounted drives will\n\
be copied to the clipboard and not displayed, do you wish to continue?\n\
Note: It may take up to a minute to extract all data.";

LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		if (device)
		{
			UINT width = LOWORD(lparam);
			UINT height = HIWORD(lparam);
			if (width != 0 && height != 0 &&
				(width != present.BackBufferWidth || height != present.BackBufferHeight))
			{
				nk_d3d9_release();
				present.BackBufferWidth = width;
				present.BackBufferHeight = height;
				HRESULT hr = IDirect3DDevice9_Reset(device, &present);
				nk_d3d9_resize(width, height);
			}
		}
		break;
	}

	if (nk_d3d9_handle_event(wnd, msg, wparam, lparam))
		return 0;

	return DefWindowProcW(wnd, msg, wparam, lparam);
}

void create_d3d9_device(HWND wnd)
{
	HRESULT hr;

	present.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	present.BackBufferWidth = WINDOW_WIDTH;
	present.BackBufferHeight = WINDOW_HEIGHT;
	present.BackBufferFormat = D3DFMT_X8R8G8B8;
	present.BackBufferCount = 1;
	present.MultiSampleType = D3DMULTISAMPLE_NONE;
	present.SwapEffect = D3DSWAPEFFECT_DISCARD;
	present.hDeviceWindow = wnd;
	present.EnableAutoDepthStencil = TRUE;
	present.AutoDepthStencilFormat = D3DFMT_D24S8;
	present.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	present.Windowed = TRUE;

	{/* first try to create Direct3D9Ex device if possible (on Windows 7+) */
		typedef HRESULT WINAPI Direct3DCreate9ExPtr(UINT, IDirect3D9Ex * *);
		Direct3DCreate9ExPtr* Direct3DCreate9ExFn = (Direct3DCreate9ExPtr*)GetProcAddress(GetModuleHandleA("d3d9.dll"), "Direct3DCreate9Ex");
		if (Direct3DCreate9ExFn) {
			IDirect3D9Ex* d3d9ex;
			if (SUCCEEDED(Direct3DCreate9ExFn(D3D_SDK_VERSION, &d3d9ex))) {
				hr = IDirect3D9Ex_CreateDeviceEx(d3d9ex, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd,
					D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_FPU_PRESERVE,
					&present, NULL, &deviceEx);
				if (SUCCEEDED(hr)) {
					device = (IDirect3DDevice9*)deviceEx;
				}
				else {
					/* hardware vertex processing not supported, no big deal
					retry with software vertex processing */
					hr = IDirect3D9Ex_CreateDeviceEx(d3d9ex, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd,
						D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_FPU_PRESERVE,
						&present, NULL, &deviceEx);
					if (SUCCEEDED(hr)) {
						device = (IDirect3DDevice9*)deviceEx;
					}
				}
				IDirect3D9Ex_Release(d3d9ex);
			}
		}
	}

	if (!device) {
		/* otherwise do regular D3D9 setup */
		IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

		hr = IDirect3D9_CreateDevice(d3d9, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_FPU_PRESERVE,
			&present, &device);
		if (FAILED(hr)) {
			/* hardware vertex processing not supported, no big deal
			retry with software vertex processing */
			hr = IDirect3D9_CreateDevice(d3d9, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_FPU_PRESERVE,
				&present, &device);
		}
		IDirect3D9_Release(d3d9);
	}
}

int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR    lpCmdLine,
	int       nCmdShow)
{
	struct nk_context* ctx;
	struct nk_colorf bg;

	/* Create window */
	WNDCLASSW wc;
	RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	DWORD exstyle = WS_EX_APPWINDOW;
	HWND wnd;
	int running = 1;

	memset(&wc, 0, sizeof(wc));
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandleW(0);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = L"eat";
	RegisterClassW(&wc);

	AdjustWindowRectEx(&rect, style, FALSE, exstyle);

	wnd = CreateWindowExW(exstyle, wc.lpszClassName, L"Execution Artifacts Tool",
		style | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		NULL, NULL, wc.hInstance, NULL);

	create_d3d9_device(wnd);

	/* GUI */
	ctx = nk_d3d9_init(device, WINDOW_WIDTH, WINDOW_HEIGHT);

	{struct nk_font_atlas* atlas;
	nk_d3d9_font_stash_begin(&atlas);
	nk_d3d9_font_stash_end();}

	/* Runtime vars */
	std::string output_str = "Nothing here yet....";
	std::string usn_journal_data = "";
	int         cur_len = output_str.length();
	bool        interactable = true;
	bool        prev_interactable = interactable;

	auto copy_to_clipboard = [&](std::string& data) {
		if (OpenClipboard(wnd) && EmptyClipboard()) {
			if (HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, data.size())) {
				memcpy(GlobalLock(hmem), data.data(), data.size());
				SetClipboardData(CF_TEXT, hmem);
				GlobalFree(hmem);
			}
			CloseClipboard();
		}
	};

	bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
	while (running)
	{
		/* Input */
		MSG msg;
		nk_input_begin(ctx);
		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				running = 0;
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		nk_input_end(ctx);

		/* GUI */
		if (nk_begin(ctx, "Execution Artifacts Tool", nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR))
		{
			/* Check if the interactable state was changed back on by UsnJournal query. */
			if (prev_interactable != interactable && interactable == true) {
				copy_to_clipboard(usn_journal_data);
				output_str = "Read " + std::to_string(usn_journal_data.size() / 1024 / 1024) + "MB from UsnJournal..";
				usn_journal_data = "";
			}
			prev_interactable = interactable;

			/* Button wrapper to enable non-immediate operations with the interactable boolean. */
			auto button = [&interactable, ctx](const char* name) {
				return nk_button_label(ctx, name) && interactable;
			};

			nk_layout_row_dynamic(ctx, 25, 6);
			if (button("Usn Journal")) {
				if (MessageBoxA(wnd, g_usn_warning_message, "Warning", MB_ICONWARNING | MB_OKCANCEL) == 1) {
					interactable = false;
					ea::get_usn_journal_info_deferred(usn_journal_data, interactable);
					output_str = "Querying UsnJournal....";
				}
			}

			if (button("UserAssist")) {
				output_str = ea::get_user_assist_info();
			}

			if (button("AppCompatFlags")) {
				output_str = ea::get_app_compat_flags_info();
			}

			if (button("MUI Cache")) {
				output_str = ea::get_mui_cache_info();
			}

			if (button("RecentApps")) {
				output_str = ea::get_recent_apps_info();
			}

			if (button("ShimCache")) {
				output_str = ea::get_shim_cache_info();
			}

			cur_len = output_str.length();;

			nk_layout_row_dynamic(ctx, 30, 1);
			nk_label(ctx, "Output:", NK_TEXT_CENTERED);

			nk_layout_row_dynamic(ctx, 492, 1);
			nk_edit_string(ctx, NK_EDIT_SELECTABLE | NK_EDIT_MULTILINE | NK_EDIT_ALLOW_TAB | NK_EDIT_CLIPBOARD,
				output_str.data(), &cur_len, output_str.size(), nk_filter_default);

			nk_layout_row_static(ctx, 25, 160, 1);
			if (nk_button_label(ctx, "Copy to Clipboard") && interactable) {
				copy_to_clipboard(output_str);
			}
		}
		nk_end(ctx);

		/* Draw */
		{
			HRESULT hr;
			hr = IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
				D3DCOLOR_COLORVALUE(bg.r, bg.g, bg.b, bg.a), 0.0f, 0);

			hr = IDirect3DDevice9_BeginScene(device);
			nk_d3d9_render(NK_ANTI_ALIASING_ON);
			hr = IDirect3DDevice9_EndScene(device);

			if (deviceEx) {
				hr = IDirect3DDevice9Ex_PresentEx(deviceEx, NULL, NULL, NULL, NULL, 0);
			}
			else {
				hr = IDirect3DDevice9_Present(device, NULL, NULL, NULL, NULL);
			}
			if (hr == D3DERR_DEVICELOST || hr == D3DERR_DEVICEHUNG || hr == D3DERR_DEVICEREMOVED) {
				/* to recover from this, you'll need to recreate device and all the resources */
				MessageBoxW(NULL, L"D3D9 device is lost or removed!", L"Error", 0);
				break;
			}
			else if (hr == S_PRESENT_OCCLUDED) {
				/* window is not visible, so vsync won't work. Let's sleep a bit to reduce CPU usage */
				Sleep(10);
			}
		}
	}
	nk_d3d9_shutdown();
	if (deviceEx)IDirect3DDevice9Ex_Release(deviceEx);
	else IDirect3DDevice9_Release(device);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);
	if (!interactable)
		exit(0); /* This is not ideal but gets the job done. */
	return 0;
}