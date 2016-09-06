// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_UTIL_NATIVE_THEME_H_
#define NATIVEUI_WIN_UTIL_NATIVE_THEME_H_

// A wrapper class for working with custom XP/Vista themes provided in
// uxtheme.dll.
// For more information on visual style parts and states, see:
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/commctls/userex/topics/partsandstates.asp

#include <windows.h>
#include <uxtheme.h>  // NOLINT

#include <map>

#include "nativeui/gfx/geometry/rect.h"
#include "nativeui/gfx/geometry/size.h"
#include "nativeui/win/base_view.h"

namespace nu {

class NativeTheme {
 public:
  NativeTheme();
  ~NativeTheme();

  enum Part {
    CheckBox,
    Radio,
    Button,
    TextField,
    NumParts,
  };

  // Each structure below holds extra information needed when painting a given
  // part.

  struct ButtonExtraParams {
    bool checked;
    bool indeterminate;  // Whether the button state is indeterminate.
    bool is_default;  // Whether the button is default button.
  };

  Size GetThemePartSize(Part part, ControlState state) const;

  HRESULT PaintPushButton(HDC hdc,
                          ControlState state,
                          const Rect& rect,
                          const ButtonExtraParams& extra) const;
  HRESULT PaintRadio(HDC hdc,
                     ControlState state,
                     const Rect& rect,
                     const ButtonExtraParams& extra) const;
  HRESULT PaintCheckBox(HDC hdc,
                        ControlState state,
                        const Rect& rect,
                        const ButtonExtraParams& extra) const;

 private:
  HRESULT PaintButton(HDC hdc,
                      ControlState state,
                      const ButtonExtraParams& extra,
                      int part_id,
                      int state_id,
                      RECT* rect) const;

  // Returns a handle to the theme data.
  HANDLE GetThemeHandle(Part part) const;

  // Closes cached theme handles so we can unload the DLL or update our UI
  // for a theme change.
  void CloseHandles() const;

  typedef HRESULT (WINAPI* DrawThemeBackgroundPtr)(HANDLE theme,
                                                   HDC hdc,
                                                   int part_id,
                                                   int state_id,
                                                   const RECT* rect,
                                                   const RECT* clip_rect);
  typedef HRESULT (WINAPI* DrawThemeBackgroundExPtr)(HANDLE theme,
                                                     HDC hdc,
                                                     int part_id,
                                                     int state_id,
                                                     const RECT* rect,
                                                     const DTBGOPTS* opts);
  typedef HRESULT (WINAPI* GetThemeColorPtr)(HANDLE hTheme,
                                             int part_id,
                                             int state_id,
                                             int prop_id,
                                             COLORREF* color);
  typedef HRESULT (WINAPI* GetThemeContentRectPtr)(HANDLE hTheme,
                                                   HDC hdc,
                                                   int part_id,
                                                   int state_id,
                                                   const RECT* rect,
                                                   RECT* content_rect);
  typedef HRESULT (WINAPI* GetThemePartSizePtr)(HANDLE hTheme,
                                                HDC hdc,
                                                int part_id,
                                                int state_id,
                                                RECT* rect,
                                                int ts,
                                                SIZE* size);
  typedef HANDLE (WINAPI* OpenThemeDataPtr)(HWND window,
                                            LPCWSTR class_list);
  typedef HRESULT (WINAPI* CloseThemeDataPtr)(HANDLE theme);

  typedef void (WINAPI* SetThemeAppPropertiesPtr) (DWORD flags);
  typedef BOOL (WINAPI* IsThemeActivePtr)();
  typedef HRESULT (WINAPI* GetThemeIntPtr)(HANDLE hTheme,
                                           int part_id,
                                           int state_id,
                                           int prop_id,
                                           int *value);

  // Function pointers into uxtheme.dll.
  DrawThemeBackgroundPtr draw_theme_;
  DrawThemeBackgroundExPtr draw_theme_ex_;
  GetThemeColorPtr get_theme_color_;
  GetThemeContentRectPtr get_theme_content_rect_;
  GetThemePartSizePtr get_theme_part_size_;
  OpenThemeDataPtr open_theme_;
  CloseThemeDataPtr close_theme_;
  SetThemeAppPropertiesPtr set_theme_properties_;
  IsThemeActivePtr is_theme_active_;
  GetThemeIntPtr get_theme_int_;

  // Handle to uxtheme.dll.
  HMODULE theme_dll_;

  // A cache of open theme handles.
  mutable HANDLE theme_handles_[NumParts];

  DISALLOW_COPY_AND_ASSIGN(NativeTheme);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_UTIL_NATIVE_THEME_H_
