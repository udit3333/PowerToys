#pragma once
//#include "pch.h"
#include "Helpers.h"
#include <interface/lowlevel_keyboard_event_data.h>
#include <winrt/Windows.UI.Xaml.Controls.h>


// Enum type to store different states of the UI
enum class KeyboardManagerUIState
{
    // If set to this value then there is no keyboard manager window currently active that requires a hook
    Deactivated,
    // If set to this value then the detect key window is currently active and it requires a hook
    DetectSingleKeyRemapWindowActivated,
    // If set to this value then the detect shortcut window is currently active and it requires a hook
    DetectShortcutWindowActivated
};

// Class to store the shared state of the keyboard manager between the UI and the hook
class KeyboardManagerState
{
private:
    // State variable used to store which UI window is currently active that requires interaction with the hook
    KeyboardManagerUIState uiState;

    // Window handle for the current UI window which is active. Should be set to nullptr if UI is deactivated
    HWND currentUIWindow;

    // Vector to store the shortcut detected in the detect shortcut UI window. This is used in both the backend and the UI.
    std::vector<DWORD> detectedShortcut;

    // Store detected remap key in the remap UI window. This is used in both the backend and the UI.
    DWORD detectedRemapKey;

    // Stores the UI element which is to be updated based on the remap key entered.
    winrt::Windows::UI::Xaml::Controls::TextBlock currentSingleKeyRemapTextBlock;

    // Stores the UI element which is to be updated based on the shortcut entered
    winrt::Windows::UI::Xaml::Controls::TextBlock currentShortcutTextBlock;

public:
    // Maps which store the remappings for each of the features. The bool fields should be initalised to false. They are used to check the current state of the shortcut (i.e is that particular shortcut currently pressed down or not).
    // Stores single key remappings
    std::unordered_map<DWORD, WORD> singleKeyReMap;

    // Stores keys which need to be changed from toggle behaviour to modifier behaviour. Eg. Caps Lock
    std::unordered_map<DWORD, bool> singleKeyToggleToMod;

    // Stores the os level shortcut remappings
    std::map<std::vector<DWORD>, std::pair<std::vector<WORD>, bool>> osLevelShortcutReMap;

    // Stores the app-specific shortcut remappings. Maps application name to the shortcut map
    std::map<std::wstring, std::map<std::vector<DWORD>, std::pair<std::vector<WORD>, bool>>> appSpecificShortcutReMap;

    // Constructor
    KeyboardManagerState();

    // Function to reset the UI state members
    void ResetUIState();

    // Function to check the if the UI state matches the argument state. For states with activated windows it also checks if the window is in focus.
    bool CheckUIState(KeyboardManagerUIState state);

    // Function to set the window handle of the current UI window that is activated
    void SetCurrentUIWindow(HWND windowHandle);

    // Function to set the UI state. When a window is activated, the handle to the window can be passed in the windowHandle argument.
    void SetUIState(KeyboardManagerUIState state, HWND windowHandle = nullptr);

    // Function to clear the OS Level shortcut remapping table
    void ClearOSLevelShortcuts();

    // Function to clear the Keys remapping table
    void ClearSingleKeyRemaps();

    // Function to add a new OS level shortcut remapping
    void AddSingleKeyRemap(const DWORD& originalKey, const WORD& newRemapKey);

    // Function to add a new OS level shortcut remapping
    void AddOSLevelShortcut(const std::vector<DWORD>& originalSC, const std::vector<WORD>& newSC);

    // Function to set the textblock of the detect shortcut UI so that it can be accessed by the hook
    void ConfigureDetectShortcutUI(const winrt::Windows::UI::Xaml::Controls::TextBlock& textBlock);

    // Function to set the textblock of the detect remap key UI so that it can be accessed by the hook
    void ConfigureDetectSingleKeyRemapUI(const winrt::Windows::UI::Xaml::Controls::TextBlock& textBlock);

    // Function to update the detect shortcut UI based on the entered keys
    void UpdateDetectShortcutUI();

    // Function to update the detect remap key UI based on the entered key.
    void UpdateDetectSingleKeyRemapUI();

    // Function to return the currently detected shortcut which is displayed on the UI
    std::vector<DWORD> GetDetectedShortcut();

    // Function to return the currently detected remap key which is displayed on the UI
    DWORD GetDetectedSingleRemapKey();

    // Function which can be used in HandleKeyboardHookEvent before the single key remap event to use the UI and suppress events while the remap window is active.
    bool DetectSingleRemapKeyUIBackend(LowlevelKeyboardEvent* data);

    // Function which can be used in HandleKeyboardHookEvent before the os level shortcut remap event to use the UI and suppress events while the remap window is active.
    bool DetectShortcutUIBackend(LowlevelKeyboardEvent* data);
};