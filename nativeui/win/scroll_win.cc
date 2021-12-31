// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/scroll_win.h"

#include <tuple>

#include "nativeui/events/win/event_win.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/win/scrollbar/scrollbar.h"

namespace nu {

ScrollImpl::ScrollImpl(Scroll* delegate)
    : ContainerImpl(delegate, this, ControlType::Scroll),
      scrollbar_height_(GetSystemMetrics(SM_CXVSCROLL)),
      delegate_(delegate) {
  SetScrollbarPolicy(Scroll::Policy::Automatic, Scroll::Policy::Automatic);
}

ScrollImpl::~ScrollImpl() {}

void ScrollImpl::SetOrigin(const Vector2d& origin) {
  UpdateOrigin(origin);
  Layout();
  Invalidate();
}

void ScrollImpl::SetContentSize(const Size& size) {
  content_size_ = size;
  UpdateScrollbar();
  UpdateOrigin(origin_);
  Layout();
  Invalidate();
}

void ScrollImpl::SetScrollbarPolicy(Scroll::Policy h_policy,
                                    Scroll::Policy v_policy) {
  h_policy_ = h_policy;
  v_policy_ = v_policy;
  UpdateScrollbar();
  UpdateOrigin(origin_);
  Layout();
  Invalidate();
}

Rect ScrollImpl::GetViewportRect() const {
  Rect viewport(size_allocation());
  viewport.Inset(0, 0,
                 v_scrollbar_ ? scrollbar_height_ : 0,
                 h_scrollbar_ ? scrollbar_height_ : 0);
  return viewport;
}

void ScrollImpl::OnScroll(int x, int y) {
  if (UpdateOrigin(origin_ + Vector2d(x, y))) {
    Layout();
    Invalidate();
  }
}

void ScrollImpl::Layout() {
  if (h_scrollbar_)
    h_scrollbar_->SizeAllocate(GetScrollbarRect(false) +
                               size_allocation().OffsetFromOrigin());
  if (v_scrollbar_)
    v_scrollbar_->SizeAllocate(GetScrollbarRect(true) +
                               size_allocation().OffsetFromOrigin());

  if (delegate_->GetContentView()) {
    Size viewport_size = GetViewportRect().size();
    Rect content_alloc = Rect(size_allocation().origin() + origin_,
                              content_size_);
    if (content_alloc.width() < viewport_size.width())
      content_alloc.set_width(viewport_size.width());
    if (content_alloc.height() < viewport_size.height())
      content_alloc.set_height(viewport_size.height());
    delegate_->GetContentView()->GetNative()->SizeAllocate(content_alloc);
    delegate_->GetContentView()->Layout();
  }
}

void ScrollImpl::ForEach(const std::function<bool(ViewImpl*)>& callback,
                         bool reverse /* ignored for Scroll */) {
  callback(delegate_->GetContentView()->GetNative()) &&
  (!h_scrollbar_ || callback(h_scrollbar_.get())) &&
  v_scrollbar_ && callback(v_scrollbar_.get());
}

bool ScrollImpl::HasChild(ViewImpl* child) {
  return delegate_->GetContentView()->GetNative() == child ||
         child == h_scrollbar_.get() || child == v_scrollbar_.get();
}

void ScrollImpl::SizeAllocate(const Rect& size_allocation) {
  ViewImpl::SizeAllocate(size_allocation);
  UpdateScrollbar();
  UpdateOrigin(origin_);
  Layout();
}

void ScrollImpl::Draw(PainterWin* painter, const Rect& dirty) {
  if (h_scrollbar_)
    DrawChild(h_scrollbar_.get(), painter, dirty);
  if (v_scrollbar_)
    DrawChild(v_scrollbar_.get(), painter, dirty);

  // The scroll view must be clipped.
  painter->ClipRectPixel(
      GetViewportRect() - size_allocation().OffsetFromOrigin());
  DrawChild(delegate_->GetContentView()->GetNative(), painter, dirty);
}

bool ScrollImpl::OnMouseWheel(NativeEvent event) {
  int16_t delta = static_cast<int16_t>(HIWORD(event->w_param));
  if (event->message == WM_MOUSEWHEEL)
    OnScroll(0, delta);
  else
    OnScroll(-delta, 0);
  return true;
}

void ScrollImpl::UpdateScrollbar() {
  int width = size_allocation().width();
  int height = size_allocation().height();
  bool show_h_scrollbar = (h_policy_ == Scroll::Policy::Always) ||
                          (h_policy_ == Scroll::Policy::Automatic &&
                           width < content_size_.width());
  bool show_v_scrollbar = (v_policy_ == Scroll::Policy::Always) ||
                          (v_policy_ == Scroll::Policy::Automatic &&
                           height < content_size_.height());
  if (show_h_scrollbar != show_v_scrollbar) {
    if (show_h_scrollbar)
      show_v_scrollbar = v_policy_ != Scroll::Policy::Never &&
                         height < content_size_.height() + scrollbar_height_;
    if (show_v_scrollbar)
      show_h_scrollbar = h_policy_ != Scroll::Policy::Never &&
                         width < content_size_.width() + scrollbar_height_;
  }
  if (show_h_scrollbar && !h_scrollbar_) {
    h_scrollbar_.reset(new Scrollbar(false, this));
    h_scrollbar_->SetParent(this);
  } else if (!show_h_scrollbar) {
    h_scrollbar_.reset();
  }
  if (show_v_scrollbar && !v_scrollbar_) {
    v_scrollbar_.reset(new Scrollbar(true, this));
    v_scrollbar_->SetParent(this);
  } else if (!show_v_scrollbar) {
    v_scrollbar_.reset();
  }
}

bool ScrollImpl::UpdateOrigin(Vector2d new_origin) {
  Rect viewport = GetViewportRect();
  if (-new_origin.x() + viewport.width() > content_size_.width())
    new_origin.set_x(viewport.width() - content_size_.width());
  if (new_origin.x() > 0)
    new_origin.set_x(0);
  if (-new_origin.y() + viewport.height() > content_size_.height())
    new_origin.set_y(viewport.height() - content_size_.height());
  if (new_origin.y() > 0)
    new_origin.set_y(0);

  if (new_origin == origin_)
    return false;
  origin_ = new_origin;
  delegate_->on_scroll.Emit(delegate_);
  return true;
}

Rect ScrollImpl::GetScrollbarRect(bool vertical) const {
  if (vertical)
    return Rect(size_allocation().width() - scrollbar_height_,
                0,
                scrollbar_height_,
                size_allocation().height() -
                (h_scrollbar_ ? scrollbar_height_ : 0));
  else
    return Rect(0,
                size_allocation().height() - scrollbar_height_,
                size_allocation().width() -
                (v_scrollbar_ ? scrollbar_height_ : 0),
                scrollbar_height_);
}

///////////////////////////////////////////////////////////////////////////////
// Public Container API implementation.

void Scroll::PlatformInit() {
  TakeOverView(new ScrollImpl(this));
}

void Scroll::PlatformSetContentView(View* view) {
  auto* scroll = static_cast<ScrollImpl*>(GetNative());
  view->GetNative()->SetParent(scroll);
  view->GetNative()->set_viewport(scroll);
}

void Scroll::SetContentSize(const SizeF& size) {
  auto* scroll = static_cast<ScrollImpl*>(GetNative());
  scroll->SetContentSize(ToCeiledSize(ScaleSize(size, scroll->scale_factor())));
}

void Scroll::SetScrollPosition(float horizon, float vertical) {
  auto* scroll = static_cast<ScrollImpl*>(GetNative());
  scroll->SetOrigin(Vector2d(-horizon * scroll->scale_factor(),
                             -vertical * scroll->scale_factor()));
}

std::tuple<float, float> Scroll::GetScrollPosition() const {
  auto* scroll = static_cast<ScrollImpl*>(GetNative());
  return std::make_tuple(-scroll->origin().x() / scroll->scale_factor(),
                         -scroll->origin().y() / scroll->scale_factor());
}

std::tuple<float, float> Scroll::GetMaximumScrollPosition() const {
  auto* scroll = static_cast<ScrollImpl*>(GetNative());
  Size content = scroll->content_size();
  Size viewport = scroll->GetViewportRect().size();
  return std::make_tuple(
      (content.width() - viewport.width()) / scroll->scale_factor(),
      (content.height() - viewport.height()) / scroll->scale_factor());
}

void Scroll::SetScrollbarPolicy(Policy h_policy, Policy v_policy) {
  auto* scroll = static_cast<ScrollImpl*>(GetNative());
  scroll->SetScrollbarPolicy(h_policy, v_policy);
}

std::tuple<Scroll::Policy, Scroll::Policy> Scroll::GetScrollbarPolicy() const {
  auto* scroll = static_cast<ScrollImpl*>(GetNative());
  return std::make_tuple(scroll->h_policy(), scroll->v_policy());
}

void Scroll::SubscribeOnScroll() {
}

}  // namespace nu
