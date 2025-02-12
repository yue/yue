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
#include <optional>

#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/rect.h"
#include "nativeui/gfx/geometry/size.h"

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

  // True if Windows supports dark mode. This does NOT indicate whether the
  // system is in dark mode, only that it is supported by this version of
  // Windows.
  const bool supports_windows_dark_mode_;

  // Handle to uxtheme.dll.
  HMODULE theme_dll_ = NULL;

  // A cache of open theme handles.
  mutable HANDLE theme_handles_[static_cast<size_t>(Part::Count)];
};

}  // namespace nu

#endif  // NATIVEUI_GFX_WIN_NATIVE_THEME_H_
