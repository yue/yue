// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/view_win.h"

#include "nativeui/events/event.h"
#include "nativeui/events/win/event_win.h"
#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/gfx/screen.h"
#include "nativeui/label.h"
#include "nativeui/win/scroll_win.h"

namespace nu {

ViewImpl::ViewImpl(ControlType type, View* delegate)
    : type_(type), scale_factor_(GetScaleFactor()), delegate_(delegate) {}

void ViewImpl::SizeAllocate(const Rect& size_allocation) {
  if (size_allocation == size_allocation_)
    return;

  Invalidate(size_allocation_);  // old
  size_allocation_ = size_allocation;
  Invalidate(size_allocation_);  // new
}

void ViewImpl::SetParent(ViewImpl* parent) {
  window_ = parent ? parent->window_ : nullptr;

  if (parent) {
    if (parent->type() == ControlType::Scroll &&
        type() != ControlType::Scrollbar)
      viewport_ = static_cast<ScrollImpl*>(parent);
    else
      viewport_ = parent->viewport_;
  } else {
    viewport_ = nullptr;
  }

  ParentChanged();
}

void ViewImpl::BecomeContentView(WindowImpl* parent) {
  window_ = parent;
  viewport_ = nullptr;

  ParentChanged();
}

void ViewImpl::Invalidate(const Rect& dirty) {
  // Nothing to draw?
  if (!window_ || size_allocation_.size().IsEmpty() || dirty.IsEmpty())
    return;

  // Can not invalidate outside the viewport.
  Rect clipped_dirty(dirty);
  if (viewport_)
    clipped_dirty.Intersect(GetClippedRect());

  if (clipped_dirty.IsEmpty())
    return;

  RECT rect = clipped_dirty.ToRECT();
  InvalidateRect(window_->hwnd(), &rect, TRUE);
}

void ViewImpl::SetFocus(bool focus) {
  is_focused_ = focus;
  Invalidate();
}

bool ViewImpl::HasFocus() const {
  return is_focused_;
}

void ViewImpl::SetVisible(bool visible) {
  is_visible_ = visible;
}

void ViewImpl::SetBackgroundColor(Color color) {
  background_color_ = color;
  Invalidate();
}

void ViewImpl::Draw(PainterWin* painter, const Rect& dirty) {
  if (!background_color_.transparent()) {
    painter->SetColor(background_color_);
    painter->FillRectPixel(dirty);
  }
}

bool ViewImpl::OnMouseClick(NativeEvent event) {
  MouseEvent client_event(event, this);
  if (client_event.type == EventType::MouseDown &&
      delegate()->on_mouse_down.Emit(delegate(), client_event))
    return true;
  if (client_event.type == EventType::MouseUp &&
      delegate()->on_mouse_up.Emit(delegate(), client_event))
    return true;
  return false;
}

bool ViewImpl::OnKeyEvent(NativeEvent event) {
  KeyEvent client_event(event, this);
  if (client_event.type == EventType::KeyDown &&
      delegate()->on_key_down.Emit(delegate(), client_event))
    return true;
  if (client_event.type == EventType::KeyUp &&
      delegate()->on_key_up.Emit(delegate(), client_event))
    return true;
  // Pass to parent if this view ignores the event.
  if (delegate()->GetParent())
    return delegate()->GetParent()->GetNative()->OnKeyEvent(event);
  return false;
}

Point ViewImpl::GetMousePosition() const {
  if (!window_)
    return Point();
  POINT p;
  ::GetCursorPos(&p);
  ::ScreenToClient(window_->hwnd(), &p);
  return Point(p) - size_allocation().OffsetFromOrigin();
}

Rect ViewImpl::GetClippedRect() const {
  Rect rect(size_allocation());
  if (viewport_)
    rect.Intersect(viewport_->GetViewportRect());
  return rect;
}

void ViewImpl::Invalidate() {
  Invalidate(size_allocation_);
}

void ViewImpl::ParentChanged() {
  // Scale the bounds after moving to a new parent.
  float new_scale_factor = window_ ? window_->scale_factor() : scale_factor_;
  if (new_scale_factor != scale_factor_) {
    size_allocation_ =
        ToNearestRect(ScaleRect(RectF(size_allocation_),
                                new_scale_factor / scale_factor_));
    scale_factor_ = new_scale_factor;
    OnDPIChanged();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public View API implementation.

void View::PlatformDestroy() {
  delete view_;
}

void View::TakeOverView(NativeView view) {
  view_ = view;
}

void View::SetBounds(const RectF& bounds) {
  SetPixelBounds(
      ToNearestRect(ScaleRect(bounds, GetNative()->scale_factor())));
}

RectF View::GetBounds() const {
  return ScaleRect(RectF(GetPixelBounds()), 1.0f / GetNative()->scale_factor());
}

void View::SetPixelBounds(const Rect& bounds) {
  Rect size_allocation(bounds);
  if (GetParent()) {
    size_allocation +=
        GetParent()->GetNative()->size_allocation().OffsetFromOrigin();
  }
  GetNative()->SizeAllocate(size_allocation);
}

Rect View::GetPixelBounds() const {
  Rect bounds(GetNative()->size_allocation());
  if (GetParent())
    bounds -= GetParent()->GetNative()->size_allocation().OffsetFromOrigin();
  return bounds;
}

void View::SchedulePaint() {
  GetNative()->Invalidate();
}

void View::PlatformSetVisible(bool visible) {
  GetNative()->SetVisible(visible);
}

bool View::IsVisible() const {
  return GetNative()->is_visible();
}

void View::Focus() {
  if (GetNative()->window())
    GetNative()->window()->focus_manager()->TakeFocus(this);
}

bool View::HasFocus() const {
  return GetNative()->HasFocus();
}

void View::SetFocusable(bool focusable) {
  GetNative()->set_focusable(focusable);
}

bool View::IsFocusable() const {
  return GetNative()->is_focusable();
}

void View::PlatformSetBackgroundColor(Color color) {
  GetNative()->SetBackgroundColor(color);
}

}  // namespace nu
