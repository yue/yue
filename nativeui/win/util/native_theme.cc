// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/util/native_theme.h"

#include <stddef.h>
#include <vsstyle.h>
#include <vssym32.h>

#include "base/win/scoped_hdc.h"

namespace nu {

namespace {

int GetWindowsPart(NativeTheme::Part part) {
  if (part == NativeTheme::CheckBox)
    return BP_CHECKBOX;
  else if (part == NativeTheme::Radio)
    return BP_RADIOBUTTON;
  else if (part == NativeTheme::Button)
    return BP_PUSHBUTTON;
  else
    return 0;
}

int GetWindowsState(NativeTheme::Part part, ControlState state) {
  switch (part) {
    case NativeTheme::CheckBox:
      switch (state) {
        case ControlState::Disabled:
          return CBS_UNCHECKEDDISABLED;
        case ControlState::Hovered:
          return CBS_UNCHECKEDHOT;
        case ControlState::Normal:
          return CBS_UNCHECKEDNORMAL;
        case ControlState::Pressed:
          return CBS_UNCHECKEDPRESSED;
        case ControlState::Size:
          NOTREACHED();
          return 0;
      }
    case NativeTheme::Button:
      switch (state) {
        case ControlState::Disabled:
          return PBS_DISABLED;
        case ControlState::Hovered:
          return PBS_HOT;
        case ControlState::Normal:
          return PBS_NORMAL;
        case ControlState::Pressed:
          return PBS_PRESSED;
        case ControlState::Size:
          NOTREACHED();
          return 0;
      }
    case NativeTheme::Radio:
      switch (state) {
        case ControlState::Disabled:
          return RBS_UNCHECKEDDISABLED;
        case ControlState::Hovered:
          return RBS_UNCHECKEDHOT;
        case ControlState::Normal:
          return RBS_UNCHECKEDNORMAL;
        case ControlState::Pressed:
          return RBS_UNCHECKEDPRESSED;
        case ControlState::Size:
          NOTREACHED();
          return 0;
      }
    default:
      return 0;
  }
}

}  // namespace

NativeTheme::NativeTheme()
    : draw_theme_(nullptr),
      draw_theme_ex_(nullptr),
      get_theme_color_(nullptr),
      get_theme_content_rect_(nullptr),
      get_theme_part_size_(nullptr),
      open_theme_(nullptr),
      close_theme_(nullptr),
      set_theme_properties_(nullptr),
      is_theme_active_(nullptr),
      get_theme_int_(nullptr),
      theme_dll_(LoadLibrary(L"uxtheme.dll")) {
  if (theme_dll_) {
    draw_theme_ = reinterpret_cast<DrawThemeBackgroundPtr>(
        GetProcAddress(theme_dll_, "DrawThemeBackground"));
    draw_theme_ex_ = reinterpret_cast<DrawThemeBackgroundExPtr>(
        GetProcAddress(theme_dll_, "DrawThemeBackgroundEx"));
    get_theme_color_ = reinterpret_cast<GetThemeColorPtr>(
        GetProcAddress(theme_dll_, "GetThemeColor"));
    get_theme_content_rect_ = reinterpret_cast<GetThemeContentRectPtr>(
        GetProcAddress(theme_dll_, "GetThemeBackgroundContentRect"));
    get_theme_part_size_ = reinterpret_cast<GetThemePartSizePtr>(
        GetProcAddress(theme_dll_, "GetThemePartSize"));
    open_theme_ = reinterpret_cast<OpenThemeDataPtr>(
        GetProcAddress(theme_dll_, "OpenThemeData"));
    close_theme_ = reinterpret_cast<CloseThemeDataPtr>(
        GetProcAddress(theme_dll_, "CloseThemeData"));
    set_theme_properties_ = reinterpret_cast<SetThemeAppPropertiesPtr>(
        GetProcAddress(theme_dll_, "SetThemeAppProperties"));
    is_theme_active_ = reinterpret_cast<IsThemeActivePtr>(
        GetProcAddress(theme_dll_, "IsThemeActive"));
    get_theme_int_ = reinterpret_cast<GetThemeIntPtr>(
        GetProcAddress(theme_dll_, "GetThemeInt"));
  }
  memset(theme_handles_, 0, sizeof(theme_handles_));
}

NativeTheme::~NativeTheme() {
  if (theme_dll_) {
    CloseHandles();
    FreeLibrary(theme_dll_);
  }
}

Size NativeTheme::GetThemePartSize(Part part, ControlState state) const {
  base::win::ScopedGetDC screen_dc(NULL);
  HANDLE handle = GetThemeHandle(part);
  if (handle && get_theme_part_size_) {
    SIZE size;
    int part_id = GetWindowsPart(part);
    int state_id = GetWindowsState(part, state);
    if (SUCCEEDED(get_theme_part_size_(handle, screen_dc, part_id, state_id,
                                       nullptr, TS_TRUE, &size)))
      return Size(size.cx, size.cy);
  }

  return (part == CheckBox || part == Radio) ?  Size(13, 13) : Size();
}

HRESULT NativeTheme::PaintPushButton(HDC hdc,
                                     ControlState state,
                                     const Rect& rect,
                                     const ButtonExtraParams& extra) const {
  int state_id = extra.is_default ? PBS_DEFAULTED : PBS_NORMAL;
  switch (state) {
    case ControlState::Disabled:
      state_id = PBS_DISABLED;
      break;
    case ControlState::Hovered:
      state_id = PBS_HOT;
      break;
    case ControlState::Normal:
      break;
    case ControlState::Pressed:
      state_id = PBS_PRESSED;
      break;
    case ControlState::Size:
      NOTREACHED();
      break;
  }

  RECT rect_win = rect.ToRECT();
  return PaintButton(hdc, state, extra, BP_PUSHBUTTON, state_id, &rect_win);
}

HRESULT NativeTheme::PaintRadio(HDC hdc,
                                ControlState state,
                                const Rect& rect,
                                const ButtonExtraParams& extra) const {
  int state_id = extra.checked ? RBS_CHECKEDNORMAL : RBS_UNCHECKEDNORMAL;
  switch (state) {
    case ControlState::Disabled:
      state_id = extra.checked ? RBS_CHECKEDDISABLED : RBS_UNCHECKEDDISABLED;
      break;
    case ControlState::Hovered:
      state_id = extra.checked ? RBS_CHECKEDHOT : RBS_UNCHECKEDHOT;
      break;
    case ControlState::Normal:
      break;
    case ControlState::Pressed:
      state_id = extra.checked ? RBS_CHECKEDPRESSED : RBS_UNCHECKEDPRESSED;
      break;
    case ControlState::Size:
      NOTREACHED();
      break;
  }

  RECT rect_win = rect.ToRECT();
  return PaintButton(hdc, state, extra, BP_RADIOBUTTON, state_id, &rect_win);
}

HRESULT NativeTheme::PaintCheckBox(HDC hdc,
                                   ControlState state,
                                   const Rect& rect,
                                   const ButtonExtraParams& extra) const {
  int state_id = extra.checked ?
      CBS_CHECKEDNORMAL :
      (extra.indeterminate ? CBS_MIXEDNORMAL : CBS_UNCHECKEDNORMAL);
  switch (state) {
    case ControlState::Disabled:
      state_id = extra.checked ?
          CBS_CHECKEDDISABLED :
          (extra.indeterminate ? CBS_MIXEDDISABLED : CBS_UNCHECKEDDISABLED);
      break;
    case ControlState::Hovered:
      state_id = extra.checked ?
          CBS_CHECKEDHOT :
          (extra.indeterminate ? CBS_MIXEDHOT : CBS_UNCHECKEDHOT);
      break;
    case ControlState::Normal:
      break;
    case ControlState::Pressed:
      state_id = extra.checked ?
          CBS_CHECKEDPRESSED :
          (extra.indeterminate ? CBS_MIXEDPRESSED : CBS_UNCHECKEDPRESSED);
      break;
    case ControlState::Size:
      NOTREACHED();
      break;
  }

  RECT rect_win = rect.ToRECT();
  return PaintButton(hdc, state, extra, BP_CHECKBOX, state_id, &rect_win);
}

HRESULT NativeTheme::PaintButton(HDC hdc,
                                 ControlState state,
                                 const ButtonExtraParams& extra,
                                 int part_id,
                                 int state_id,
                                 RECT* rect) const {
  HANDLE handle = GetThemeHandle(Button);
  if (handle && draw_theme_)
    return draw_theme_(handle, hdc, part_id, state_id, rect, NULL);

  // Adjust classic_state based on part, state, and extras.
  int classic_state = 0;
  switch (part_id) {
    case BP_CHECKBOX:
      classic_state |= DFCS_BUTTONCHECK;
      break;
    case BP_RADIOBUTTON:
      classic_state |= DFCS_BUTTONRADIO;
      break;
    case BP_PUSHBUTTON:
      classic_state |= DFCS_BUTTONPUSH;
      break;
    default:
      NOTREACHED();
      break;
  }

  switch (state) {
    case ControlState::Disabled:
      classic_state |= DFCS_INACTIVE;
      break;
    case ControlState::Hovered:
    case ControlState::Normal:
      break;
    case ControlState::Pressed:
      classic_state |= DFCS_PUSHED;
      break;
    case ControlState::Size:
      NOTREACHED();
      break;
  }

  if (extra.checked)
    classic_state |= DFCS_CHECKED;

  // Draw it manually.
  // All pressed states have both low bits set, and no other states do.
  const bool focused = ((state_id & ETS_FOCUSED) == ETS_FOCUSED);
  const bool pressed = ((state_id & PBS_PRESSED) == PBS_PRESSED);
  if ((BP_PUSHBUTTON == part_id) && (pressed || focused)) {
    // BP_PUSHBUTTON has a focus rect drawn around the outer edge, and the
    // button itself is shrunk by 1 pixel.
    HBRUSH brush = GetSysColorBrush(COLOR_3DDKSHADOW);
    if (brush) {
      FrameRect(hdc, rect, brush);
      InflateRect(rect, -1, -1);
    }
  }
  DrawFrameControl(hdc, rect, DFC_BUTTON, classic_state);

  // Draw the focus rectangle (the dotted line box) only on buttons.  For radio
  // and checkboxes, we let webkit draw the focus rectangle (orange glow).
  if ((BP_PUSHBUTTON == part_id) && focused) {
    // The focus rect is inside the button.  The exact number of pixels depends
    // on whether we're in classic mode or using uxtheme.
    if (handle && get_theme_content_rect_) {
      get_theme_content_rect_(handle, hdc, part_id, state_id, rect, rect);
    } else {
      InflateRect(rect, -GetSystemMetrics(SM_CXEDGE),
                  -GetSystemMetrics(SM_CYEDGE));
    }
    DrawFocusRect(hdc, rect);
  }

  // Classic theme doesn't support indeterminate checkboxes.  We draw
  // a recangle inside a checkbox like IE10 does.
  if (part_id == BP_CHECKBOX && extra.indeterminate) {
    RECT inner_rect = *rect;
    // "4 / 13" is same as IE10 in classic theme.
    int padding = (inner_rect.right - inner_rect.left) * 4 / 13;
    InflateRect(&inner_rect, -padding, -padding);
    int color_index = state == ControlState::Disabled ? COLOR_GRAYTEXT :
                                                        COLOR_WINDOWTEXT;
    FillRect(hdc, &inner_rect, GetSysColorBrush(color_index));
  }

  return S_OK;
}

HANDLE NativeTheme::GetThemeHandle(Part part) const {
  if (!open_theme_ || part < 0 || part >= NumParts)
    return 0;

  // Translate part to real theme names.
  switch (part) {
    case CheckBox:
    case Radio:
      part = Button;
      break;
    default:
      break;
  }

  if (theme_handles_[part])
    return theme_handles_[part];

  // Not found, try to load it.
  HANDLE handle = 0;
  switch (part) {
    case Button:
      handle = open_theme_(NULL, L"Button");
      break;
    case TextField:
      handle = open_theme_(NULL, L"Edit");
      break;
    default:
      NOTREACHED();
      break;
  }
  theme_handles_[part] = handle;
  return handle;
}

void NativeTheme::CloseHandles() const {
  if (!close_theme_)
    return;

  for (int i = 0; i < NumParts; ++i) {
    if (theme_handles_[i]) {
      close_theme_(theme_handles_[i]);
      theme_handles_[i] = nullptr;
    }
  }
}

}  // namespace nu
