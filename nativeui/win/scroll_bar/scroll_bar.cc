// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scroll_bar/scroll_bar.h"

#include "nativeui/state.h"

namespace nu {

ScrollBarView::ScrollBarView(bool vertical, ScrollView* scroll)
    : ContainerView(this, ControlType::Scroll),
      theme_(State::current()->GetNativeTheme()),
      vertical_(true),
      scroll_(scroll) {}

ScrollBarView::~ScrollBarView() {
}

void ScrollBarView::Layout() {
}

std::vector<View*> ScrollBarView::GetChildren() {
  return std::vector<View*>();
}

void ScrollBarView::Draw(PainterWin* painter, const Rect& dirty) {
  HDC dc = painter->GetHDC();

  theme_->PaintScrollbarTrack(
      dc, vertical_, state(),
      Rect(size_allocation().size()) + painter->origin(), params_);

  painter->ReleaseHDC(dc);
}

}  // namespace nu
