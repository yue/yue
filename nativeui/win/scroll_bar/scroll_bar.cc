// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scroll_bar/scroll_bar.h"

#include "nativeui/state.h"

namespace nu {

ScrollBarView::ScrollBarView(bool vertical, ScrollView* scroll)
    : ContainerView(this, ControlType::ScrollBar),
      theme_(State::current()->GetNativeTheme()),
      near_button_(vertical ? ScrollBarButton::Up : ScrollBarButton::Left,
                   scroll),
      far_button_(vertical ? ScrollBarButton::Down : ScrollBarButton::Right,
                  scroll),
      vertical_(vertical),
      scroll_(scroll) {
  near_button_.SetParent(this);
  far_button_.SetParent(this);
}

ScrollBarView::~ScrollBarView() {
}

void ScrollBarView::Layout() {
  int box = vertical_ ? size_allocation().width() : size_allocation().height();

  Rect near_allocation(size_allocation());
  if (vertical_)
    near_allocation.set_height(
        std::min(box,
                 static_cast<int>(std::ceil(near_allocation.height() / 2))));
  else
    near_allocation.set_width(
        std::min(box,
                 static_cast<int>(std::ceil(near_allocation.width() / 2))));
  near_button_.SizeAllocate(near_allocation);

  Rect far_allocation(near_allocation);
  if (vertical_)
    far_allocation.set_y(
        std::max(size_allocation().height() - near_allocation.height(),
                 size_allocation().height() - box));
  else
    far_allocation.set_x(
        std::max(size_allocation().width() - near_allocation.width(),
                 size_allocation().width() - box));
  far_button_.SizeAllocate(far_allocation);
}

std::vector<BaseView*> ScrollBarView::GetChildren() {
  return std::vector<BaseView*>{&near_button_, &far_button_};
}

void ScrollBarView::Draw(PainterWin* painter, const Rect& dirty) {
  Rect track_area(size_allocation());
  track_area.Subtract(near_button_.size_allocation());
  track_area.Subtract(far_button_.size_allocation());
  if (!track_area.IsEmpty()) {
    HDC dc = painter->GetHDC();
    theme_->PaintScrollbarTrack(dc, vertical_, state(), track_area, params_);
    painter->ReleaseHDC(dc);
  }

  ContainerView::Draw(painter, dirty);
}

}  // namespace nu
