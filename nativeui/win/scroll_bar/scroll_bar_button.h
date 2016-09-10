// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SCROLL_BAR_SCROLL_BAR_BUTTON_H_
#define NATIVEUI_WIN_SCROLL_BAR_SCROLL_BAR_BUTTON_H_

#include "nativeui/win/scroll_win.h"
#include "nativeui/win/util/native_theme.h"

namespace nu {

class ScrollBarButton : public BaseView {
 public:
  enum Type {
    Up,
    Down,
    Left,
    Right,
  };

  ScrollBarButton(Type type, ScrollView* scroll);
  ~ScrollBarButton() override;

  // BaseView:
  void Draw(PainterWin* painter, const Rect& dirty) override;

 private:
  NativeTheme* theme_;
  NativeTheme::ScrollbarArrowExtraParams params_ = {0};

  Type type_;
  ScrollView* scroll_;  // weak ref
};

}  // namespace nu

#endif  // NATIVEUI_WIN_SCROLL_BAR_SCROLL_BAR_BUTTON_H_
