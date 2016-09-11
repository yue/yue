// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SCROLL_BAR_SCROLL_BAR_THUMB_H_
#define NATIVEUI_WIN_SCROLL_BAR_SCROLL_BAR_THUMB_H_

#include "nativeui/win/util/native_theme.h"

namespace nu {

class ScrollBarView;

class ScrollBarThumb : public BaseView {
 public:
  ScrollBarThumb(bool vertical, ScrollBarView* scroll_bar);
  ~ScrollBarThumb() override;

  // BaseView:
  void OnMouseEnter() override;
  void OnMouseLeave() override;
  bool OnMouseClick(UINT message, UINT flags, const Point& point) override;
  void Draw(PainterWin* painter, const Rect& dirty) override;

  NativeTheme::ScrollbarThumbExtraParams* params() { return &params_; }

 private:
  NativeTheme* theme_;
  NativeTheme::ScrollbarThumbExtraParams params_ = {0};

  bool vertical_;
  ScrollBarView* scroll_bar_;  // weak ref
};

}  // namespace nu

#endif  // NATIVEUI_WIN_SCROLL_BAR_SCROLL_BAR_THUMB_H_
