// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scrollbar/scrollbar.h"

#include <algorithm>

#include "nativeui/events/win/event_win.h"

namespace nu {

Scrollbar::Scrollbar(bool vertical, ScrollImpl* scroll)
    : ContainerImpl(nullptr, this, ControlType::Scrollbar),
      near_button_(vertical ? ScrollbarButton::Up : ScrollbarButton::Left,
                   this),
      far_button_(vertical ? ScrollbarButton::Down : ScrollbarButton::Right,
                  this),
      thumb_(vertical, this),
      repeater_([this] { this->OnClick(); }),
      vertical_(vertical),
      scroll_(scroll) {
  near_button_.SetParent(this);
  far_button_.SetParent(this);
  thumb_.SetParent(this);
}

Scrollbar::~Scrollbar() {
}

void Scrollbar::LineUp() {
  if (vertical_)
    scroll_->OnScroll(0, GetLineHeight());
  else
    scroll_->OnScroll(GetLineHeight(), 0);
}

void Scrollbar::LineDown() {
  if (vertical_)
    scroll_->OnScroll(0, -GetLineHeight());
  else
    scroll_->OnScroll(-GetLineHeight(), 0);
}

void Scrollbar::PageUp() {
  if (vertical_)
    scroll_->OnScroll(0, GetPageHeight());
  else
    scroll_->OnScroll(GetPageHeight(), 0);
}

void Scrollbar::PageDown() {
  if (vertical_)
    scroll_->OnScroll(0, -GetPageHeight());
  else
    scroll_->OnScroll(-GetPageHeight(), 0);
}

int Scrollbar::GetValue() const {
  if (vertical_)
    return thumb_.size_allocation().y() -
           near_button_.size_allocation().bottom();
  else
    return thumb_.size_allocation().x() -
           near_button_.size_allocation().right();
}

void Scrollbar::SetValue(int value) {
  int thumb_size = thumb_.GetSize();
  int track_size = GetTrackSize();
  if (track_size == thumb_size)
    return;
  int offset = (value * (contents_size_ - viewport_size_)) /
               (track_size - thumb_size);
  Vector2d origin(scroll_->origin());
  if (vertical_)
    origin.set_y(-offset);
  else
    origin.set_x(-offset);
  scroll_->SetOrigin(origin);
}

void Scrollbar::Layout() {
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
        std::max(size_allocation().bottom() - near_allocation.height(),
                 size_allocation().bottom() - box_size));
  else
    far_allocation.set_x(
        std::max(size_allocation().right() - near_allocation.width(),
                 size_allocation().right() - box_size));
  far_button_.SizeAllocate(far_allocation);

  UpdateThumbPosition();
}

void Scrollbar::ForEach(const std::function<bool(ViewImpl*)>& callback,
                        bool reverse /* ignored for Scrollbar */) {
  callback(&near_button_) && callback(&far_button_) && callback(&thumb_);
}

bool Scrollbar::HasChild(ViewImpl* child) {
  return child == &near_button_ || child == &far_button_ || child == &thumb_;
}

void Scrollbar::OnMouseEnter(NativeEvent event) {
  SetState(ControlState::Hovered);
  near_button_.params()->is_hovering = true;
  far_button_.params()->is_hovering = true;
  thumb_.params()->is_hovering = true;
  ContainerImpl::OnMouseEnter(event);
}

void Scrollbar::OnMouseLeave(NativeEvent event) {
  repeater_.Stop();

  SetState(ControlState::Normal);
  near_button_.params()->is_hovering = false;
  far_button_.params()->is_hovering = false;
  thumb_.params()->is_hovering = false;
  ContainerImpl::OnMouseLeave(event);
}

bool Scrollbar::OnMouseClick(NativeEvent event) {
  repeater_.Stop();
  if (ContainerImpl::OnMouseClick(event))
    return true;
  if (event->message == WM_LBUTTONDOWN) {
    OnClick();
    repeater_.Start();
    return true;
  }
  return false;
}

void Scrollbar::Draw(PainterWin* painter, const Rect& dirty) {
  int track_size = GetTrackSize();
  if (track_size > 0) {
    int box_size = GetBoxSize();
    Rect track_area(vertical_ ? 0 : box_size, vertical_ ? box_size : 0,
                    vertical_ ? box_size : track_size,
                    vertical_ ? track_size : box_size);
    NativeTheme::ExtraParams params;
    params.scrollbar_track = params_;
    painter->DrawNativeTheme(
        (vertical_ ? NativeTheme::Part::ScrollbarVerticalTrack
                   : NativeTheme::Part::ScrollbarHorizontalTrack),
        state(),
        track_area,
        params);
  }

  ContainerImpl::Draw(painter, dirty);
}

void Scrollbar::UpdateThumbPosition() {
  // The size of contents and viewport.
  Size content_size = scroll_->content_size();
  contents_size_ = vertical_ ? content_size.height() : content_size.width();
  Rect viewport(scroll_->GetViewportRect());
  viewport_size_ = vertical_ ? viewport.height() : viewport.width();

  // Make sure contents_size is always > 0 to avoid divide by zero errors in
  // calculations throughout this code.
  contents_size_ = std::max(1, contents_size_);
  viewport_size_ = std::max(1, viewport_size_);

  // Calculate the size of thumb button.
  double ratio =
      std::min(1.0, static_cast<double>(viewport_size_) / contents_size_);
  int track_size = GetTrackSize();
  int thumb_size = static_cast<int>(ratio * track_size);

  // Calculate the position of thumb button.
  int thumb_max = track_size - thumb_size;
  int scroll_amount = GetScrollAmout();
  int thumb_pos = (scroll_amount + viewport_size_ == contents_size_) ?
      thumb_max :
      ((scroll_amount * thumb_max) / (contents_size_ - viewport_size_));

  // Put the thumb button.
  int box_size = GetBoxSize();
  Rect rect(vertical_ ? 0 : (box_size + thumb_pos),
            vertical_ ? (box_size + thumb_pos) : 0,
            vertical_ ? box_size : thumb_size,
            vertical_ ? thumb_size : box_size);
  thumb_.SizeAllocate(rect + size_allocation().OffsetFromOrigin());
}

void Scrollbar::OnClick() {
  Rect thumb(thumb_.size_allocation() - size_allocation().OffsetFromOrigin());
  Point cursor(GetMousePosition());
  if (thumb.Contains(cursor))
    return;
  if ((vertical_ && cursor.y() < thumb.y()) ||
      (!vertical_ && cursor.x() < thumb.x()))
    PageUp();
  else
    PageDown();
}

int Scrollbar::GetTrackSize() const {
  return vertical_ ?
      size_allocation().height() - near_button_.size_allocation().height()
                                 - far_button_.size_allocation().height() :
      size_allocation().width() - near_button_.size_allocation().width()
                                - far_button_.size_allocation().width();
}

int Scrollbar::GetBoxSize() const {
  return vertical_ ? size_allocation().width() : size_allocation().height();
}

int Scrollbar::GetScrollAmout() const {
  return vertical_ ? -scroll_->origin().y() : -scroll_->origin().x();
}

int Scrollbar::GetLineHeight() const {
  int max_amout = std::ceil(20 * scale_factor());
  View* contents = scroll_->delegate()->GetContentView();
  if (contents->GetClassName() == Container::kClassName) {
    Container* container = static_cast<Container*>(contents);
    if (container->ChildCount() > 0)
      return std::min(max_amout,
                      container->ChildAt(0)->GetPixelBounds().height());
  }
  return max_amout;
}

int Scrollbar::GetPageHeight() const {
  if (vertical_)
    return scroll_->GetViewportRect().height();
  else
    return scroll_->GetViewportRect().width();
}

}  // namespace nu
