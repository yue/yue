// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/view_win.h"

#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/label.h"
#include "nativeui/win/scroll_win.h"

namespace nu {

void ViewImpl::SizeAllocate(const Rect& size_allocation) {
  if (size_allocation == size_allocation_)
    return;

  Invalidate(size_allocation_);  // old
  size_allocation_ = size_allocation;
  Invalidate(size_allocation_);  // new
}

void ViewImpl::SetParent(ViewImpl* parent) {
  float old_scale_factor = scale_factor();

  window_ = parent ? parent->window_ : nullptr;

  if (parent) {
    if (parent->type() == ControlType::Scroll &&
        type() != ControlType::ScrollBar)
      viewport_ = static_cast<ScrollView*>(parent);
    else
      viewport_ = parent->viewport_;
  } else {
    viewport_ = nullptr;
  }

  ParentChanged(old_scale_factor);
}

void ViewImpl::BecomeContentView(WindowImpl* parent) {
  float old_scale_factor = scale_factor();

  window_ = parent;
  viewport_ = nullptr;

  ParentChanged(old_scale_factor);
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

bool ViewImpl::IsFocused() const {
  return is_focused_;
}

void ViewImpl::SetBackgroundColor(Color color) {
  background_color_ = color;
  Invalidate();
}

void ViewImpl::Draw(PainterWin* painter, const Rect& dirty) {
  painter->SetColor(background_color_);
  painter->FillPixelRect(RectF(dirty));
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

void ViewImpl::ParentChanged(float old_scale_factor) {
  // Scale the bounds after moving to a new parent.
  if (old_scale_factor != scale_factor()) {
    size_allocation_ = ScaleToEnclosingRect(size_allocation_,
                                            scale_factor() / old_scale_factor);
    preferred_size_ = ScaleToCeiledSize(preferred_size_,
                                        scale_factor() / old_scale_factor);
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
  SetPixelBounds(ToEnclosingRect(ScaleRect(bounds,
                                 GetNative()->scale_factor())));
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

void View::PlatformSetVisible(bool visible) {
  GetNative()->set_visible(visible);
}

bool View::IsVisible() const {
  return GetNative()->is_visible();
}

void View::PlatformSetBackgroundColor(Color color) {
  GetNative()->SetBackgroundColor(color);
}

}  // namespace nu
