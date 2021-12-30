// Copyright 2016 Cheng Zhao. All rights reserved.
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_WIN_NATIVE_THEME_H_
#define NATIVEUI_GFX_WIN_NATIVE_THEME_H_

// A wrapper class for working with custom XP/Vista themes provided in
// uxtheme.dll.
// For more information on visual style parts and states, see:
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/commctls/userex/topics/partsandstates.asp

#include <windows.h>
#include <uxtheme.h>  // NOLINT

#include <map>

#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/rect.h"
#include "nativeui/gfx/geometry/size.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

// Structurs used by private win32 APIs.
enum PreferredAppMode { Default, AllowDark, ForceDark, ForceLight, Max };
enum WINDOWCOMPOSITIONATTRIB { WCA_USEDARKMODECOLORS = 26 };
struct WINDOWCOMPOSITIONATTRIBDATA {
  WINDOWCOMPOSITIONATTRIB Attrib;
  PVOID pvData;
  SIZE_T cbData;
};

namespace nu {

// The state of the control.
enum class ControlState {
  // IDs defined as specific values for use in arrays.
  Disabled = 0,
  Hovered,
  Normal,
  Pressed,
  Size,
};

class NativeTheme {
 public:
  NativeTheme();
  ~NativeTheme();

  NativeTheme& operator=(const NativeTheme&) = delete;
  NativeTheme(const NativeTheme&) = delete;

  enum class Part {
    Checkbox,
    Radio,
    Button,

    ScrollbarUpArrow,
    ScrollbarDownArrow,
    ScrollbarLeftArrow,
    ScrollbarRightArrow,

    ScrollbarHorizontalThumb,
    ScrollbarVerticalThumb,
    ScrollbarHorizontalGripper,
    ScrollbarVerticalGripper,
    ScrollbarHorizontalTrack,
    ScrollbarVerticalTrack,

    TabPanel,
    TabItem,

    Count,
  };

  // Each structure below holds extra information needed when painting a given
  // part.

  struct ButtonExtraParams {
    bool checked;
    bool indeterminate;  // Whether the button state is indeterminate.
    bool is_default;  // Whether the button is default button.
  };

  struct ScrollbarArrowExtraParams {
    bool is_hovering;
  };

  struct ScrollbarThumbExtraParams {
    bool is_hovering;
  };

  struct ScrollbarTrackExtraParams {
    bool is_upper;
    int track_x;
    int track_y;
    int track_width;
    int track_height;
  };

  union ExtraParams {
    ButtonExtraParams button;
    ScrollbarArrowExtraParams scrollbar_arrow;
    ScrollbarThumbExtraParams scrollbar_thumb;
    ScrollbarTrackExtraParams scrollbar_track;
  };

  // Initialize dark mode related functions.
  bool InitializeDarkMode();

  // Return whether dark mode is supported.
  bool IsDarkModeSupported() const;

  // Set whether to allow dark mode for the app.
  void SetAppDarkModeEnabled(bool enable);

  // Enable dark mode for the window.
  void EnableDarkModeForWindow(HWND hwnd);

  // Return whether app is using dark mode.
  bool IsAppDarkMode() const;

  // Return whether system is using dark mode.
  bool IsSystemDarkMode() const;

  // Return the size that a control takes.
  Size GetThemePartSize(HDC hdc, Part part, ControlState state) const;

  // Paint the control into |hdc|.
  void Paint(Part part, HDC hdc, ControlState state, const Rect& rect,
             const ExtraParams& extra);

 private:
  HRESULT PaintPushButton(HDC hdc,
                          ControlState state,
                          const Rect& rect,
                          const ButtonExtraParams& extra) const;
  HRESULT PaintRadio(HDC hdc,
                     ControlState state,
                     const Rect& rect,
                     const ButtonExtraParams& extra) const;
  HRESULT PaintCheckbox(HDC hdc,
                        ControlState state,
                        const Rect& rect,
                        const ButtonExtraParams& extra) const;
  HRESULT PaintScrollbarArrow(Part part,
                              HDC hdc,
                              ControlState state,
                              const Rect& rect,
                              const ScrollbarArrowExtraParams& extra) const;
  HRESULT PaintScrollbarThumb(Part part,
                              HDC hdc,
                              ControlState state,
                              const Rect& rect,
                              const ScrollbarThumbExtraParams& extra) const;
  HRESULT PaintScrollbarTrack(Part part,
                              HDC hdc,
                              ControlState state,
                              const Rect& rect,
                              const ScrollbarTrackExtraParams& extra) const;
  HRESULT PaintTabPanel(Part part,
                        HDC hdc,
                        ControlState state,
                        const Rect& rect) const;
  HRESULT PaintTabItem(Part part,
                       HDC hdc,
                       ControlState state,
                       const Rect& rect) const;

  HRESULT PaintButton(HDC hdc,
                      ControlState state,
                      const ButtonExtraParams& extra,
                      int part_id,
                      int state_id,
                      RECT* rect) const;

  // Paints a theme part, with support for scene scaling in high-DPI mode.
  // |theme| is the theme handle. |hdc| is the handle for the device context.
  // |part_id| is the identifier for the part (e.g. thumb gripper). |state_id|
  // is the identifier for the rendering state of the part (e.g. hover). |rect|
  // is the bounds for rendering, expressed in logical coordinates.
  HRESULT PaintScaledTheme(HANDLE theme,
                           HDC hdc,
                           int part_id,
                           int state_id,
                           const Rect& rect) const;

  // Returns a handle to the theme data.
  HANDLE GetThemeHandle(Part part) const;

  // Closes cached theme handles so we can unload the DLL or update our UI
  // for a theme change.
  void CloseHandles() const;

  using OpenNcThemeDataPtr = HTHEME (WINAPI*)(HWND, LPCWSTR);    //NOLINT
  using ShouldAppsUseDarkModePtr = bool (WINAPI*)();             //NOLINT
  using AllowDarkModeForWindowPtr = bool (WINAPI*)(HWND, bool);  //NOLINT
  using AllowDarkModeForAppPtr = bool (WINAPI*)(bool);           //NOLINT
  using RefreshImmersiveColorPolicyStatePtr = void (WINAPI*)();  //NOLINT
  using SetPreferredAppModePtr = PreferredAppMode (WINAPI*)(PreferredAppMode);                  //NOLINT
  using SetWindowCompositionAttributePtr = BOOL (WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);  //NOLINT

  // Function pointers into uxtheme.dll.
  OpenNcThemeDataPtr open_nc_theme_date_ = nullptr;
  ShouldAppsUseDarkModePtr should_app_use_dark_mode_ = nullptr;
  AllowDarkModeForWindowPtr allow_dark_mode_for_window_ = nullptr;
  AllowDarkModeForAppPtr allow_dark_mode_for_app_ = nullptr;
  SetPreferredAppModePtr set_preferred_app_mode_ = nullptr;
  RefreshImmersiveColorPolicyStatePtr refresh_color_policy_ = nullptr;
  SetWindowCompositionAttributePtr set_window_attribute_ = nullptr;

  // Whether dark mode is supported.
  absl::optional<bool> dark_mode_supported_;

  // Handle to uxtheme.dll.
  HMODULE theme_dll_ = NULL;

  // A cache of open theme handles.
  mutable HANDLE theme_handles_[static_cast<size_t>(Part::Count)];
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_NATIVE_THEME_H_
