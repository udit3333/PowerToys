#pragma once
#include <optional>
#include <string>
#include <Windows.h>
#include <string>

// Returns RECT with positions of the minmize/maximize buttons of the given window.
// Does not always work, since some apps draw custom toolbars.
std::optional<RECT> get_button_pos(HWND hwnd);
// Gets position of given window.
std::optional<RECT> get_window_pos(HWND hwnd);
// Gets mouse postion.
std::optional<POINT> get_mouse_pos();
// Gets active window, filtering out all "non standard" windows like the taskbar, etc.
HWND get_filtered_active_window();
// Gets window ancestor (usualy the window we want to do stuff with), filtering out all "non standard" windows like the taskbar, etc. and provide the app process path
struct WindowAndProcPath {
  HWND hwnd = nullptr;
  std::wstring process_path;
};
WindowAndProcPath get_filtered_base_window_and_path(HWND window);

// Calculate sizes
int width(const RECT& rect);
int height(const RECT& rect);
// Compare rects
bool operator<(const RECT& lhs, const RECT& rhs);
// Moves and/or resizes small_rect to fit inside big_rect.
RECT keep_rect_inside_rect(const RECT& small_rect, const RECT& big_rect);
// Initializes and runs windows message loop
int run_message_loop();

void show_last_error_message(LPCWSTR lpszFunction, DWORD dw);

enum WindowState {
  UNKNONW,
  MINIMIZED,
  MAXIMIZED,
  SNAPED_TOP_LEFT,
  SNAPED_LEFT,
  SNAPED_BOTTOM_LEFT,
  SNAPED_TOP_RIGHT,
  SNAPED_RIGHT,
  SNAPED_BOTTOM_RIGHT,
  RESTORED
};
WindowState get_window_state(HWND hwnd);

// Returns true if the current process is running with elevated privileges
bool is_process_elevated();

// Drops the elevated privilages if present
bool drop_elevated_privileges();

// Run command as elevated user, returns true if succeeded
bool run_elevated(const std::wstring& file, const std::wstring& params);

// Run command as non-elevated user, returns true if succeeded
bool run_non_elevated(const std::wstring& file, const std::wstring& params);

// Run command with the same elevation, returns true if succedded
bool run_same_elevation(const std::wstring& file, const std::wstring& params);

// Get the executable path or module name for modern apps
std::wstring get_process_path(DWORD pid) noexcept;
// Get the executable path or module name for modern apps
std::wstring get_process_path(HWND hwnd) noexcept;

std::wstring get_product_version();

std::wstring get_module_filename(HMODULE mod = nullptr);
std::wstring get_module_folderpath(HMODULE mod = nullptr);

// Get a string from the resource file
std::wstring get_resource_string(UINT resource_id, HINSTANCE instance, const wchar_t* fallback);
// Wrapper for getting a string from the resource file. Returns the resource id text when fails.
// Requires that
//  extern "C" IMAGE_DOS_HEADER __ImageBase;
// is added to the .cpp file.
#define GET_RESOURCE_STRING(resource_id) get_resource_string(resource_id, reinterpret_cast<HINSTANCE>(&__ImageBase), L#resource_id)
