#include "platform/platform.h"

// If not on windows, not include the code
#if KPLATFORM_WINDOWS

#include "core/logger.h"
#include "core/input.h"
#include "core/event.h"

#include "renderer/vulkan/vulkan_platform.h"
#include "containers/darray.h"

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "renderer/vulkan/vulkan_types.inl"

typedef struct internal_state {
	HINSTANCE h_instance;
	HWND hwnd;
	VkSurfaceKHR surface;
} internal_state;

static f64 clock_frequency;
static LARGE_INTEGER start_time;

LRESULT CALLBACK win32_process_message(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);

b8 platform_startup(
	platform_state* platform_state,
	const char* application_name,
	i32 x,
	i32 y,
	i32 width,
	i32 height) {
	
	platform_state->internal_state = malloc(sizeof(internal_state));
	internal_state *state = (internal_state *)platform_state->internal_state;

	state->h_instance = GetModuleHandleA(0);

	HICON icon = LoadIcon(state->h_instance, IDI_APPLICATION);
	WNDCLASSA wc;
	memset(&wc, 0, sizeof(wc));
	wc.style = CS_DBLCLKS; // Get double-click 
	wc.lpfnWndProc = win32_process_message;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = state->h_instance;
	wc.hIcon = icon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszClassName = "moteur_test_window_class";

	if (!RegisterClassA(&wc)) {
		MessageBoxA(0, "Window registration failed", "Error", MB_ICONERROR | MB_OK);
		return false;
	}

	// Create the window
	u32 client_x = x;
	u32 client_y = y;
	u32 client_width = width;
	u32 client_height = height;

	u32 window_x = client_x;
	u32 window_y = client_y;
	u32 window_width = client_width;
	u32 window_height = client_height;

	u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
	u32 window_ex_style = WS_EX_APPWINDOW;

	window_style |= WS_MINIMIZEBOX;
	window_style |= WS_MAXIMIZEBOX;
	window_style |= WS_THICKFRAME;

	// Obtain the size of the non-client area
	RECT window_rect = { 0, 0, 0, 0};
	AdjustWindowRectEx(&window_rect, window_style, 0, window_ex_style);

	window_x += window_rect.left;
	window_y += window_rect.top;

	// Grow the window size to compensate for the non-client area
	window_width += window_rect.right - window_rect.left;
	window_height += window_rect.bottom - window_rect.top;

	HWND handle = CreateWindowExA(
		window_ex_style,
		wc.lpszClassName,
		application_name,
		window_style,
		window_x,
		window_y,
		window_width,
		window_height,
		0,
		0,
		state->h_instance,
		0
	);

	if (!handle) {
		MessageBoxA(0, "Window creation failed", "Error", MB_ICONERROR | MB_OK);
		KFATAL("Window creation failed");
		return false;
	} else {
		state->hwnd = handle;
	}

	b32 should_activate = true;
	i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;

	ShowWindow(state->hwnd, show_window_command_flags);


	// Clock setup
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	KINFO("Clock frequency: %f", (f64)frequency.QuadPart);
	clock_frequency = 1.0 / (f64)frequency.QuadPart;
	QueryPerformanceCounter(&start_time);
	KINFO("Clock start time: %f", (f64)start_time.QuadPart);


	return true;
}

void platform_shutdown(platform_state* platform_state) {
	internal_state *state = (internal_state *)platform_state->internal_state;
	if (state->hwnd) {
		DestroyWindow(state->hwnd);
		free(platform_state->internal_state);
		state->hwnd = 0;
	}
}

b8 platform_pump_messages(platform_state* platform_state) {
	MSG msg;
	while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	return true;
}

void* platform_allocate(u64 size, b8 aligned) {
	return aligned ? _aligned_malloc(size, 16) : malloc(size);
}

void platform_free(void* block, b8 aligned) {
	if (aligned) {
		_aligned_free(block);
	} else {
		free(block);
	}
}

void* platform_zero_memory(void* block, u64 size) {
	return memset(block, 0, size);
}

void* platform_copy_memory(void* dest, const void* source, u64 size) {
	return memcpy(dest, source, size);
}

void* platform_set_memory(void* dest, i32 value, u64 size) {
	return memset(dest, value, size);
}

void platform_console_write(const char* message, u8 color) {
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	// FATAL, ERROR, WARN, INFO, DEBUG, TRACE
	static u8 levels[6] = {64, 4, 6, 2, 1, 8};
	SetConsoleTextAttribute(console_handle, levels[color]);
	OutputDebugStringA(message);
	u64 length = strlen(message);
	LPDWORD number_written = 0;
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
}

void platform_console_write_error(const char* message, u8 color) {
	HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
	// FATAL, ERROR, WARN, INFO, DEBUG, TRACE
	static u8 levels[6] = {64, 4, 6, 2, 1, 8};
	SetConsoleTextAttribute(console_handle, levels[color]);
	OutputDebugStringA(message);
	u64 length = strlen(message);
	LPDWORD number_written = 0;
	WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
}

f64 platform_get_absolute_time() {
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	return (f64)(current_time.QuadPart - start_time.QuadPart) * clock_frequency;
}

void platform_sleep(u64 ms) {
	Sleep((DWORD)ms);
}

// Required extensions for Vulkan on Windows
void platform_get_required_extension_names(const char*** extensions) {
	darray_push(*extensions, &"VK_KHR_win32_surface");
}

// Surface creation for Vulkan
b8 platform_create_vulkan_surface(platform_state *plat_state, vulkan_context *context) {
    // Simply cold-cast to the known type.
    internal_state *state = (internal_state *)plat_state->internal_state;

    VkWin32SurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    create_info.hinstance = state->h_instance;
    create_info.hwnd = state->hwnd;

    VkResult result = vkCreateWin32SurfaceKHR(context->instance, &create_info, context->allocator, &state->surface);
    if (result != VK_SUCCESS) {
        KFATAL("Vulkan surface creation failed.");
        return false;
    }

    context->surface = state->surface;
    return true;
}

LRESULT CALLBACK win32_process_message(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
	switch (msg) {
		case WM_ERASEBKGND:
			// Tell the OS that we will handle the background painting
			return 1;
		case WM_CLOSE:
			event_context data = {};
			event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);
			return true;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_SIZE:
		{
			//Get the updated window size
			RECT client_rect;
			GetClientRect(hwnd, &client_rect);
			u32 width = client_rect.right - client_rect.left;
			u32 height = client_rect.bottom - client_rect.top;

			// Fire the resize event
			event_context context;
			context.data.u16[0] = (u16)width;
			context.data.u16[1] = (u16)height;
			event_fire(EVENT_CODE_RESIZED, 0, context);
		} break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
			// Key pressed/released
			b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
			keys key = (u16)w_param;

			// Alt key
            if (w_param == VK_MENU) {
                if (GetKeyState(VK_RMENU) & 0x8000) {
                    key = KEY_RALT;
                } else if (GetKeyState(VK_LMENU) & 0x8000) {
                    key = KEY_LALT;
                }
            } else if (w_param == VK_SHIFT) {
                if (GetKeyState(VK_RSHIFT) & 0x8000) {
                    key = KEY_RSHIFT;
                } else if (GetKeyState(VK_LSHIFT) & 0x8000) {
                    key = KEY_LSHIFT;
                }
            } else if (w_param == VK_CONTROL) {
                if (GetKeyState(VK_RCONTROL) & 0x8000) {
                    key = KEY_RCONTROL;
                } else if (GetKeyState(VK_LCONTROL) & 0x8000) {
                    key = KEY_LCONTROL;
                }
            }

			// Pass to the input subsystem for processing
			input_process_key(key, pressed);
		case WM_MOUSEMOVE:
		{
			//Mouse move
			i32 x_position = GET_X_LPARAM(l_param);
			i32 y_position = GET_Y_LPARAM(l_param);
			input_process_mouse_move(x_position, y_position);
		}
		case WM_MOUSEWHEEL:
		{
			i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
			if (z_delta != 0) {
				// Flatten the input to an OS-independent value (-1, 1)
				z_delta = (z_delta < 0) ? -1 : 1;
			}
			input_process_mouse_wheel(z_delta);
		}
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN;
			buttons mouse_button = BUTTON_MAX_BUTTONS; 
			switch (msg) {
				case WM_LBUTTONDOWN:
				case WM_LBUTTONUP:
					mouse_button = BUTTON_LEFT;
					break;
				case WM_MBUTTONDOWN:
				case WM_MBUTTONUP:
					mouse_button = BUTTON_MIDDLE;
					break;
				case WM_RBUTTONDOWN:
				case WM_RBUTTONUP:
					mouse_button = BUTTON_RIGHT;
					break;
			}
			if (mouse_button != BUTTON_MAX_BUTTONS) {
				input_process_button(mouse_button, pressed);
			}
		} break;
	}
	return DefWindowProcA(hwnd, msg, w_param, l_param);
}

#endif // KPLATFORM_WINDOWS