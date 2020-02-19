#pragma once
#include <interface/powertoy_module_interface.h>
#include <interface/lowlevel_keyboard_event_data.h>
#include "overlay_window.h"

#include "resource.h"

// We support only one instance of the overlay
extern class OverlayWindow* instance;

class TargetState;

class OverlayWindow : public PowertoyModuleIface
{
public:
    OverlayWindow();
    virtual const wchar_t* get_name() override;
    virtual const wchar_t** get_events() override;
    virtual bool get_config(wchar_t* buffer, int* buffer_size) override;

    virtual void set_config(const wchar_t* config) override;
    virtual void enable() override;
    virtual void disable() override;
    virtual bool is_enabled() override;
    virtual intptr_t signal_event(const wchar_t* name, intptr_t data) override;

    virtual void register_system_menu_helper(PowertoySystemMenuIface* helper) override {}
    virtual void signal_system_menu_action(const wchar_t* name) override {}

    void on_held();
    void on_held_press(DWORD vkCode);
    void quick_hide();
    void was_hidden();

    virtual void destroy() override;

private:
    std::wstring app_name;
    std::unique_ptr<TargetState> target_state;
    std::unique_ptr<D2DOverlayWindow> winkey_popup;
    bool _enabled = false;

    void init_settings();
    void disable(bool trace_event);

    struct PressTime
    {
        PCWSTR name = L"press_time";
        int value = 900; // ms
        int resourceId = IDS_SETTING_DESCRIPTION_PRESS_TIME;
    } pressTime;

    struct OverlayOpacity
    {
        PCWSTR name = L"overlay_opacity";
        int value = 90; // percent
        int resourceId = IDS_SETTING_DESCRIPTION_OVERLAY_OPACITY;
    } overlayOpacity;

    struct Theme
    {
        PCWSTR name = L"theme";
        std::wstring value = L"system";
        int resourceId = IDS_SETTING_DESCRIPTION_THEME;
        std::vector<std::pair<std::wstring, UINT>> keys_and_texts = {
            { L"system", IDS_SETTING_DESCRIPTION_THEME_SYSTEM },
            { L"light", IDS_SETTING_DESCRIPTION_THEME_LIGHT },
            { L"dark", IDS_SETTING_DESCRIPTION_THEME_DARK }
        };
    } theme;
};
