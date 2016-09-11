// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scroll_bar/scroll_bar.h"

#include "nativeui/state.h"

namespace nu {

ScrollBarView::ScrollBarView(bool vertical, Scroll* scroll)
    : ContainerView(this, ControlType::ScrollBar),
      theme_(State::current()->GetNativeTheme()),
      near_button_(vertical ? ScrollBarButton::Up : ScrollBarButton::Left,
                   this),
      far_button_(vertical ? ScrollBarButton::Down : ScrollBarButton::Right,
                  this),
      thumb_(vertical, this),
      vertical_(vertical),
      scroll_(scroll) {
  near_button_.SetParent(this);
  far_button_.SetParent(this);
  thumb_.SetParent(this);
}

ScrollBarView::~ScrollBarView() {
}

void ScrollBarView::UpdateThumbPosition() {
  // The size of contents and viewport.
  int contents_len = vertical_ ?
      scroll_->GetContentView()->view()->size_allocation().height() :
      scroll_->GetContentView()->view()->size_allocation().width();
  Rect viewport(static_cast<ScrollView*>(scroll_->view())->GetViewportRect());
  int viewport_len = vertical_ ? viewport.height() : viewport.width();
  if (contents_len == 0 || viewport_len == 0)
    return;

  // Calculate the size of thumb button.
  double ratio =
      std::min(1.0, static_cast<double>(viewport_len) / contents_len);
  int track_size = GetTrackSize();
  int thumb_size = static_cast<int>(ratio * track_size);

  // Calculate the position of thumb button.
  int thumb_max = track_size - thumb_size;
  int scroll_amount = GetScrollAmout();
  int thumb_pos = (scroll_amount + viewport_len == contents_len) ?
      thumb_max :
      ((scroll_amount * thumb_max) / (contents_len - viewport_len));

  // Put the thumb button.
  int box_size = GetBoxSize();
  Rect rect(vertical_ ? 0 : (box_size + thumb_pos),
            vertical_ ? (box_size + thumb_pos) : 0,
            vertical_ ? box_size : thumb_size,
            vertical_ ? thumb_size : box_size);
  thumb_.SizeAllocate(rect + size_allocation().OffsetFromOrigin());
}

void ScrollBarView::Layout() {
  int box_size = GetBoxSize();

  // Put the Up/Left arrow button.
  Rect near_allocation(size_allocation());
  if (vertical_)
    near_allocation.set_height(
        std::min(box_size,
                 static_cast<int>(std::ceil(near_allocation.height() / 2))));
  else
    near_allocation.set_width(
        std::min(box_size,
                 static_cast<int>(std::ceil(near_allocation.width() / 2))));
  near_button_.SizeAllocate(near_allocation);

  // Put the Down/Right arrow button.
  Rect far_allocation(near_allocation);
  if (vertical_)
    far_allocation.set_y(
        std::max(size_allocation().height() - near_allocation.height(),
                 size_allocation().height() - box_size));
  else
    far_allocation.set_x(
        std::max(size_allocation().width() - near_allocation.width(),
                 size_allocation().width() - box_size));
  far_button_.SizeAllocate(far_allocation);

  UpdateThumbPosition();
}

std::vector<BaseView*> ScrollBarView::GetChildren() {
  return std::vector<BaseView*>{&near_button_, &far_button_, &thumb_};
}

void ScrollBarView::Draw(PainterWin* painter, const Rect& dirty) {
  int track_size = GetTrackSize();
  if (track_size > 0) {
    HDC dc = painter->GetHDC();
    int box_size = GetBoxSize();
    Rect track_area(vertical_ ? 0 : box_size, vertical_ ? box_size : 0,
                    vertical_ ? box_size : track_size,
                    vertical_ ? track_size : box_size);
    theme_->PaintScrollbarTrack(dc, vertical_, state(),
                                track_area + painter->origin(), params_);
    painter->ReleaseHDC(dc);
  }

  ContainerView::Draw(painter, dirty);
}

int ScrollBarView::GetTrackSize() const {
  return vertical_ ?
      size_allocation().height() - near_button_.size_allocation().height()
                                 - far_button_.size_allocation().height() :
      size_allocation().width() - near_button_.size_allocation().width()
                                - far_button_.size_allocation().width();
}

int ScrollBarView::GetBoxSize() const {
  return vertical_ ? size_allocation().width() : size_allocation().height();
}

int ScrollBarView::GetScrollAmout() const {
  auto* scroll_view = static_cast<ScrollView*>(scroll_->view());
  return vertical_ ? -scroll_view->origin().y() : -scroll_view->origin().x();
}

}  // namespace nu
