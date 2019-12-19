#include "pch.h"
#include <interface/powertoy_module_interface.h>
#include <interface/lowlevel_keyboard_event_data.h>
#include <interface/win_hook_event_data.h>
#include <common/settings_objects.h>
#include "trace.h"
#include <shobjidl.h> 


extern "C" IMAGE_DOS_HEADER __ImageBase;
void launch_new_window();

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void create_window();
HWND window_handle = nullptr;
HMODULE hnd;
PWSTR filePath;

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
    // create_window();
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* pFileOpen;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr))
        {
            // Show the Open dialog box.
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Display the file name to the user.
                    if (SUCCEEDED(hr))
                    {
                        MessageBox(NULL, pszFilePath, L"File Path", MB_OK);
                        wcscpy_s(filePath, wcslen(pszFilePath) + 1, pszFilePath);
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }
    return 1;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Trace::RegisterProvider();
        hnd = hModule;
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        Trace::UnregisterProvider();
        break;
    }
    return TRUE;
}

// The PowerToy name that will be shown in the settings.
const static wchar_t* MODULE_NAME = L"SamplePowerToy";
// Add a description that will we shown in the module settings page.
const static wchar_t* MODULE_DESC = L"<Power Toy Sample>";

// These are the properties shown in the Settings page.
struct ModuleSettings
{
    // Add the PowerToy module properties with default values.
    // Currently available types:
    // - int
    // - bool
    // - string

    //bool bool_prop = true;
    //int int_prop = 10;
    //std::wstring string_prop = L"The quick brown fox jumps over the lazy dog";
    //std::wstring color_prop = L"#1212FF";

} g_settings;

// Implement the PowerToy Module Interface and all the required methods.
class SamplePowerToy : public PowertoyModuleIface
{
private:
    // The PowerToy state.
    bool m_enabled = false;

    // Load initial settings from the persisted values.
    void init_settings();

	bool check_supress(LowlevelKeyboardEvent data);

public:
    // Constructor
    SamplePowerToy()
    {
        init_settings();
    };

    // Destroy the powertoy and free memory
    virtual void destroy() override
    {
        delete this;
    }

    // Return the display name of the powertoy, this will be cached by the runner
    virtual const wchar_t* get_name() override
    {
        return MODULE_NAME;
    }

    // Return array of the names of all events that this powertoy listens for, with
    // nullptr as the last element of the array. Nullptr can also be retured for empty
    // list.
    virtual const wchar_t** get_events() override
    {
        static const wchar_t* events[] = { ll_keyboard, nullptr };
        // Available events:
        // - ll_keyboard
        // - win_hook_event
        //
        // static const wchar_t* events[] = { ll_keyboard,
        //                                   win_hook_event,
        //                                   nullptr };

        return events;
    }

    // Return JSON with the configuration options.
    virtual bool get_config(wchar_t* buffer, int* buffer_size) override
    {
        HINSTANCE hinstance = reinterpret_cast<HINSTANCE>(&__ImageBase);

        // Create a Settings object.
        PowerToysSettings::Settings settings(hinstance, get_name());
        settings.set_description(MODULE_DESC);

        // Show an overview link in the Settings page
        //settings.set_overview_link(L"https://");

        // Show a video link in the Settings page.
        //settings.set_video_link(L"https://");

        // A bool property with a toggle editor.
        //settings.add_bool_toogle(
        //  L"bool_toggle_1", // property name.
        //  L"This is what a BoolToggle property looks like", // description or resource id of the localized string.
        //  g_settings.bool_prop // property value.
        //);

        // An integer property with a spinner editor.
        //settings.add_int_spinner(
        //  L"int_spinner_1", // property name
        //  L"This is what a IntSpinner property looks like", // description or resource id of the localized string.
        //  g_settings.int_prop, // property value.
        //  0, // min value.
        //  100, // max value.
        //  10 // incremental step.
        //);

        // A string property with a textbox editor.
        //settings.add_string(
        //  L"string_text_1", // property name.
        //  L"This is what a String property looks like", // description or resource id of the localized string.
        //  g_settings.string_prop // property value.
        //);

        // A string property with a color picker editor.
        //settings.add_color_picker(
        //  L"color_picker_1", // property name.
        //  L"This is what a ColorPicker property looks like", // description or resource id of the localized string.
        //  g_settings.color_prop // property value.
        //);

        // A custom action property. When using this settings type, the "PowertoyModuleIface::call_custom_action()"
        // method should be overriden as well.
        //settings.add_custom_action(
        //  L"custom_action_id", // action name.
        //  L"This is what a CustomAction property looks like", // label above the field.
        //  L"Call a custom action", // button text.
        //  L"Press the button to call a custom action." // display values / extended info.
        //);

        return settings.serialize_to_buffer(buffer, buffer_size);
    }

    // Signal from the Settings editor to call a custom action.
    // This can be used to spawn more complex editors.
    virtual void call_custom_action(const wchar_t* action) override
    {
        static UINT custom_action_num_calls = 0;
        try
        {
            // Parse the action values, including name.
            PowerToysSettings::CustomActionObject action_object =
                PowerToysSettings::CustomActionObject::from_json_string(action);

            //if (action_object.get_name() == L"custom_action_id") {
            //  // Execute your custom action
            //}
        }
        catch (std::exception&)
        {
            // Improper JSON.
        }
    }

    // Called by the runner to pass the updated settings values as a serialized JSON.
    virtual void set_config(const wchar_t* config) override
    {
        try
        {
            // Parse the input JSON string.
            PowerToysSettings::PowerToyValues values =
                PowerToysSettings::PowerToyValues::from_json_string(config);

            // Update a bool property.
            //if (auto v = values.get_bool_value(L"bool_toggle_1")) {
            //  g_settings.bool_prop = *v;
            //}

            // Update an int property.
            //if (auto v = values.get_int_value(L"int_spinner_1")) {
            //  g_settings.int_prop = *v;
            //}

            // Update a string property.
            //if (auto v = values.get_string_value(L"string_text_1")) {
            //  g_settings.string_prop = *v;
            //}

            // Update a color property.
            //if (auto v = values.get_string_value(L"color_picker_1")) {
            //  g_settings.color_prop = *v;
            //}

            // If you don't need to do any custom processing of the settings, proceed
            // to persists the values calling:
            values.save_to_settings_file();
            // Otherwise call a custom function to process the settings before saving them to disk:
            // save_settings();
        }
        catch (std::exception&)
        {
            // Improper JSON.
        }
    }

    // Enable the powertoy
    virtual void enable()
    {
        m_enabled = true;
    }

    // Disable the powertoy
    virtual void disable()
    {
        m_enabled = false;
        // ShowWindow(window_handle, SW_HIDE);
    }

    // Returns if the powertoys is enabled
    virtual bool is_enabled() override
    {
        return m_enabled;
    }

    // Handle incoming event, data is event-specific
    virtual intptr_t signal_event(const wchar_t* name, intptr_t data) override
    {
        if (wcscmp(name, ll_keyboard) == 0)
        {
            auto& event = *(reinterpret_cast<LowlevelKeyboardEvent*>(data));
            // Return 1 if the keypress is to be suppressed (not forwarded to Windows),
            // otherwise return 0.
            return check_supress(event);
        }
        else if (wcscmp(name, win_hook_event) == 0)
        {
            auto& event = *(reinterpret_cast<WinHookEvent*>(data));
            // Return value is ignored
            return 0;
        }
        return 0;
    }

    // This methods are part of an experimental features not fully supported yet
    virtual void register_system_menu_helper(PowertoySystemMenuIface* helper) override
    {
    }

    virtual void signal_system_menu_action(const wchar_t* name) override
    {
    }
};

// Load the settings file.
void SamplePowerToy::init_settings()
{
    try
    {
        // Load and parse the settings file for this PowerToy.
        PowerToysSettings::PowerToyValues settings =
            PowerToysSettings::PowerToyValues::load_from_settings_file(SamplePowerToy::get_name());

        // Load a bool property.
        //if (auto v = settings.get_bool_value(L"bool_toggle_1")) {
        //  g_settings.bool_prop = *v;
        //}

        // Load an int property.
        //if (auto v = settings.get_int_value(L"int_spinner_1")) {
        //  g_settings.int_prop = *v;
        //}

        // Load a string property.
        //if (auto v = settings.get_string_value(L"string_text_1")) {
        //  g_settings.string_prop = *v;
        //}

        // Load a color property.
        //if (auto v = settings.get_string_value(L"color_picker_1")) {
        //  g_settings.color_prop = *v;
        //}
    }
    catch (std::exception&)
    {
        // Error while loading from the settings file. Let default values stay as they are.
    }
}

void create_window() 
{
    try
    {
        const wchar_t CLASS_NAME[] = L"Sample Window Class";
        HINSTANCE hinstance = reinterpret_cast<HINSTANCE>(&__ImageBase);

        WNDCLASS wc = {};

        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hnd;
        wc.lpszClassName = CLASS_NAME;

        RegisterClass(&wc);

        // Create the window.

        window_handle = CreateWindowEx(
            0, // Optional window styles.
            CLASS_NAME, // Window class
            L"Learn to Program Windows", // Window text
            WS_OVERLAPPEDWINDOW, // Window style

            // Size and position
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,

            NULL, // Parent window
            NULL, // Menu
            hnd, // Instance handle
            NULL // Additional application data
        );

        if (window_handle == NULL)
        {
            return;
        }

		ShowWindow(window_handle, SW_SHOWNORMAL);

        // Run the message loop.

        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return;
    }
    catch (std::exception ex)
    {
    }
}

bool SamplePowerToy::check_supress(LowlevelKeyboardEvent data) 
{
    if (data.wParam == WM_KEYDOWN)
    {
        bool ctrlKeyPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000);
        bool winKeyPressed = (GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000);
        if (ctrlKeyPressed && data.lParam->vkCode == 0x5A)
        {
            if (window_handle == nullptr) launch_new_window();
			return 1;
        }

		if (ctrlKeyPressed && data.lParam->vkCode == 0x4C)
        {
            
            return 1;
        }

		
        if (winKeyPressed && data.lParam->vkCode == 0x46)
        {
            if (SendNotifyMessageA(HWND_BROADCAST, WM_HOTKEY, VK_LWIN, 0x53))
                return 1;
        }

    }

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        window_handle = NULL;
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
    }
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void launch_new_window() 
{
    CreateThread(0, NULL, ThreadProc, (LPVOID)L"Window Title", NULL, NULL);
}

// This method of saving the module settings is only required if you need to do any
// custom processing of the settings before saving them to disk.
//void SamplePowerToy::save_settings() {
//  try {
//    // Create a PowerToyValues object for this PowerToy
//    PowerToysSettings::PowerToyValues values(get_name());
//
//    // Save a bool property.
//    //values.add_property(
//    //  L"bool_toggle_1", // property name
//    //  g_settings.bool_prop // property value
//    //);
//
//    // Save an int property.
//    //values.add_property(
//    //  L"int_spinner_1", // property name
//    //  g_settings.int_prop // property value
//    //);
//
//    // Save a string property.
//    //values.add_property(
//    //  L"string_text_1", // property name
//    //  g_settings.string_prop // property value
//    );
//
//    // Save a color property.
//    //values.add_property(
//    //  L"color_picker_1", // property name
//    //  g_settings.color_prop // property value
//    //);
//
//    // Save the PowerToyValues JSON to the power toy settings file.
//    values.save_to_settings_file();
//  }
//  catch (std::exception ex) {
//    // Couldn't save the settings.
//  }
//}



extern "C" __declspec(dllexport) PowertoyModuleIface* __cdecl powertoy_create()
{
    return new SamplePowerToy();
}