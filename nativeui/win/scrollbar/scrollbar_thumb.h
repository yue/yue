// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SCROLLBAR_SCROLLBAR_THUMB_H_
#define NATIVEUI_WIN_SCROLLBAR_SCROLLBAR_THUMB_H_

#include "nativeui/gfx/win/native_theme.h"
#include "nativeui/win/view_win.h"

namespace nu {

class Scrollbar;

class ScrollbarThumb : public ViewImpl {
 public:
  ScrollbarThumb(bool vertical, Scrollbar* scrollbar);
  ~ScrollbarThumb() override;

  int GetSize() const;

  // ViewImpl:
  void OnMouseEnter() override;
  void OnMouseMove(UINT flags, const Point& point) override;
  void OnMouseLeave() override;
  bool OnMouseClick(UINT message, UINT flags, const Point& point) override;
  void OnCaptureLost() override;
  void Draw(PainterWin* painter, const Rect& dirty) override;

  NativeTheme::ScrollbarThumbExtraParams* params() { return &params_; }

 private:
  NativeTheme::ScrollbarThumbExtraParams params_ = {0};

  bool is_hovering_ = false;
  bool is_capturing_ = false;

  Point pressed_point_;
  int last_value_ = 0;

  bool vertical_;
  Scrollbar* scrollbar_;  // weak ref
};

}  // namespace nu

#endif  // NATIVEUI_WIN_SCROLLBAR_SCROLLBAR_THUMB_H_
