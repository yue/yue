// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SCROLL_BAR_SCROLL_BAR_H_
#define NATIVEUI_WIN_SCROLL_BAR_SCROLL_BAR_H_

#include "nativeui/win/scroll_win.h"
#include "nativeui/win/util/native_theme.h"

namespace nu {

// Draws the native scroll bar.
class ScrollBarView : public ContainerView,
                      public ContainerView::Delegate {
 public:
  ScrollBarView(bool vertical, ScrollView* scroll);
  ~ScrollBarView() override;

  // ContainerView::Delegate:
  void Layout() override;
  std::vector<View*> GetChildren() override;

  // BaseView:
  void Draw(PainterWin* painter, const Rect& dirty) override;

 private:
  NativeTheme* theme_;
  NativeTheme::ScrollbarTrackExtraParams params_ = {0};

  bool vertical_;
  ScrollView* scroll_;  // weak ref
};

}  // namespace nu

#endif  // NATIVEUI_WIN_SCROLL_BAR_SCROLL_BAR_H_
