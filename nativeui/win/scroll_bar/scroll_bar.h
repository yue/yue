// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SCROLL_BAR_SCROLL_BAR_H_
#define NATIVEUI_WIN_SCROLL_BAR_SCROLL_BAR_H_

#include "nativeui/win/scroll_bar/scroll_bar_button.h"
#include "nativeui/win/scroll_bar/scroll_bar_thumb.h"
#include "nativeui/win/scroll_win.h"

namespace nu {

// Draws the native scroll bar.
class ScrollBarView : public ContainerView,
                      public ContainerView::Delegate {
 public:
  ScrollBarView(bool vertical, Scroll* scroll);
  ~ScrollBarView() override;

  void UpdateThumbPosition();

  // ContainerView::Delegate:
  void Layout() override;
  std::vector<BaseView*> GetChildren() override;

  // BaseView:
  void OnMouseEnter() override;
  void OnMouseLeave() override;
  void Draw(PainterWin* painter, const Rect& dirty) override;

 private:
  int GetTrackSize() const;
  int GetBoxSize() const;
  int GetScrollAmout() const;

  NativeTheme* theme_;
  NativeTheme::ScrollbarTrackExtraParams params_ = {0};

  ScrollBarButton near_button_;
  ScrollBarButton far_button_;
  ScrollBarThumb thumb_;

  bool vertical_;
  Scroll* scroll_;  // weak ref
};

}  // namespace nu

#endif  // NATIVEUI_WIN_SCROLL_BAR_SCROLL_BAR_H_
