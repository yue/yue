// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/view_win.h"

#include <utility>
#include <vector>

#include "nativeui/events/event.h"
#include "nativeui/events/win/event_win.h"
#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/gfx/screen.h"
#include "nativeui/label.h"
#include "nativeui/state.h"
#include "nativeui/System.h"
#include "nativeui/win/dragging_info_win.h"
#include "nativeui/win/scroll_win.h"

namespace nu {

ViewImpl::ViewImpl(ControlType type, View* delegate)
    : type_(type),
      font_(System::GetDefaultFont()),
      color_(System::GetColor(System::Color::Text)),
      scale_factor_(GetScaleFactor()),
      delegate_(delegate) {}

void ViewImpl::SizeAllocate(const Rect& size_allocation) {
  if (size_allocation == size_allocation_)
    return;

  bool size_changed = size_allocation.size() != size_allocation_.size();

  Invalidate(size_allocation_);  // old
  size_allocation_ = size_allocation;
  Invalidate(size_allocation_);  // new

  if (size_changed && delegate_)
    delegate_->OnSizeChanged();
}

UINT ViewImpl::HitTest(const Point& point) const {
  return draggable_ ? HTCAPTION : HTCLIENT;
}

void ViewImpl::SetParent(ViewImpl* parent) {
  if (window())
    window()->focus_manager()->RemoveFocus(this);

  window_ = parent ? parent->window_ : nullptr;
  parent_ = parent;

  if (!dragged_types_.empty() && window())
    window()->RegisterDropTarget();

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
  parent_ = nullptr;
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

void ViewImpl::ClipRectForChild(const ViewImpl* child, Rect* rect) const {
  rect->Intersect(GetClippedRect());
}

void ViewImpl::SetFocus(bool focus) {
  if (is_focused_ == focus)
    return;
  is_focused_ = focus;
  if (window()) {
    if (focus) {
      ::SetFocus(window()->hwnd());  // need this to take focus from subwin
      window()->focus_manager()->TakeFocus(this);
    } else {
      window()->focus_manager()->RemoveFocus(this);
    }
  }
  Invalidate();
}

bool ViewImpl::HasFocus() const {
  return is_focused_;
}

void ViewImpl::VisibilityChanged() {
  if (parent_)
    is_tree_visible_ = is_visible_ && parent_->is_tree_visible_;
  else
    is_tree_visible_ = is_visible_;
  if (!is_tree_visible_)  // remove focus when view is hidden
    SetFocus(false);
}

void ViewImpl::SetFont(Font* font) {
  font_ = font;
  Invalidate();
}

void ViewImpl::SetColor(Color color) {
  color_ = color;
  Invalidate();
}

void ViewImpl::SetBackgroundColor(Color color) {
  background_color_ = color;
  Invalidate();
}

void ViewImpl::RegisterDraggedTypes(std::set<Clipboard::Data::Type> types) {
  dragged_types_ = std::move(types);
  if (!dragged_types_.empty() && window())
    window()->RegisterDropTarget();
}

void ViewImpl::Draw(PainterWin* painter, const Rect& dirty) {
  if (!background_color_.transparent()) {
    painter->SetColor(background_color_);
    painter->FillRectPixel(dirty);
  }
}

void ViewImpl::OnMouseMove(NativeEvent event) {
  if (!delegate() || delegate()->on_mouse_move.IsEmpty())
    return;
  event->w_param = 0;
  delegate()->on_mouse_move.Emit(delegate(), MouseEvent(event, this));
}

void ViewImpl::OnMouseEnter(NativeEvent event) {
  if (!delegate() || delegate()->on_mouse_enter.IsEmpty())
    return;
  event->w_param = 1;
  delegate()->on_mouse_enter.Emit(delegate(), MouseEvent(event, this));
}

void ViewImpl::OnMouseLeave(NativeEvent event) {
  if (!delegate() || delegate()->on_mouse_leave.IsEmpty())
    return;
  event->w_param = 2;
  delegate()->on_mouse_leave.Emit(delegate(), MouseEvent(event, this));
}

bool ViewImpl::OnMouseClick(NativeEvent event) {
  // If the view is disabled, prevent future processes.
  if (is_disabled())
    return true;

  // Clicking a view should move the focus to it.
  // This has to be done before handling the mouse event, because user may
  // want to move focus to other view later.
  if (is_focusable() && window() && event->message == WM_LBUTTONDOWN &&
      type() != ControlType::Subwin) {  // subwin handles clicking on its own
    ::SetFocus(window()->hwnd());  // need this to take focus from subwin
    window_->focus_manager()->TakeFocus(this);
  }

  if (!delegate())
    return false;
  MouseEvent client_event(event, this);
  if (client_event.type == EventType::MouseDown &&
      delegate()->on_mouse_down.Emit(delegate(), client_event))
    return true;
  if (client_event.type == EventType::MouseUp &&
      delegate()->on_mouse_up.Emit(delegate(), client_event))
    return true;
  return false;
}

bool ViewImpl::OnSetCursor(NativeEvent event) {
  if (cursor()) {
    ::SetCursor(cursor()->GetNative());
    return true;
  }
  return false;
}

void ViewImpl::OnCaptureLost() {
  if (delegate())
    delegate()->on_capture_lost.Emit(delegate());
}

bool ViewImpl::OnKeyEvent(NativeEvent event) {
  if (!delegate())
    return false;
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
  else if (window())
    return window()->HandleKeyEvent(client_event);
  return false;
}

int ViewImpl::OnDragEnter(IDataObject* data, int effect, const Point& point) {
  if (!delegate() || !delegate()->handle_drag_enter || !AcceptsDropping(data))
    return last_drag_effect_ = DRAG_OPERATION_UNHANDLED;

  DraggingInfoWin info(data, effect);
  PointF client_point = ScalePoint(
      PointF(point - size_allocation().OffsetFromOrigin()),
      1.f / scale_factor());
  return last_drag_effect_ =
      delegate()->handle_drag_enter(delegate(), &info, client_point);
}

int ViewImpl::OnDragUpdate(IDataObject* data, int effect, const Point& point) {
  if (!AcceptsDropping(data))
    return DRAG_OPERATION_UNHANDLED;
  if (!delegate() || !delegate()->handle_drag_update)
    return last_drag_effect_;

  DraggingInfoWin info(data, effect);
  PointF client_point = ScalePoint(
      PointF(point - size_allocation().OffsetFromOrigin()),
      1.f / scale_factor());
  return last_drag_effect_ =
      delegate()->handle_drag_update(delegate(), &info, client_point);
}

void ViewImpl::OnDragLeave(IDataObject* data) {
  if (!delegate() || delegate()->on_drag_leave.IsEmpty() ||
      !AcceptsDropping(data))
    return;
  DraggingInfoWin info(data, DRAG_OPERATION_NONE);
  delegate()->on_drag_leave.Emit(delegate(), &info);
}

int ViewImpl::OnDrop(IDataObject* data, int effect, const Point& point) {
  // Emit OnDragLeave to match GTK's behavior.
  OnDragLeave(data);

  if (!delegate() || !delegate()->handle_drop || !AcceptsDropping(data))
    return DRAG_OPERATION_UNHANDLED;

  DraggingInfoWin info(data, effect);
  PointF client_point = ScalePoint(
      PointF(point - size_allocation().OffsetFromOrigin()),
      1.f / scale_factor());
  return delegate()->handle_drop(delegate(), &info, client_point) ?
      last_drag_effect_ : DRAG_OPERATION_NONE;
}

bool ViewImpl::AcceptsDropping(IDataObject* data) {
  DraggingInfoWin info(data, 0);
  for (const auto& type : dragged_types_) {
    if (info.IsDataAvailable(type))
      return true;
  }
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
  if (!is_visible_)
    return Rect();
  Rect rect(size_allocation());
  if (viewport_)
    rect.Intersect(viewport_->GetViewportRect());
  if (parent())
    parent()->ClipRectForChild(this, &rect);
  return rect;
}

void ViewImpl::SetVisible(bool visible) {
  is_visible_ = visible;
  VisibilityChanged();
  Invalidate();
}

void ViewImpl::Invalidate() {
  Invalidate(size_allocation_);
}

void ViewImpl::SetState(ControlState state) {
  state_ = state;
  Invalidate();
}

void ViewImpl::ParentChanged() {
  VisibilityChanged();
  // Scale the bounds after moving to a new parent.
  float new_scale_factor = window_ ? window_->scale_factor() : scale_factor_;
  if (new_scale_factor != scale_factor_) {
    size_allocation_ =
        ToNearestRect(ScaleRect(RectF(size_allocation_),
                                new_scale_factor / scale_factor_));
    scale_factor_ = new_scale_factor;
  }
  // Always notify when parent is changed, since certain measurements are done
  // based on the native window.
  OnDPIChanged();
}

///////////////////////////////////////////////////////////////////////////////
// Public View API implementation.

void View::PlatformDestroy() {
  delete view_;
}

void View::TakeOverView(NativeView view) {
  view_ = view;
}

Vector2dF View::OffsetFromView(const View* from) const {
  Vector2d offset = view_->size_allocation().OffsetFromOrigin() -
                    from->GetNative()->size_allocation().OffsetFromOrigin();
  return ScaleVector2d(offset, 1.f / view_->scale_factor());
}

Vector2dF View::OffsetFromWindow() const {
  return ScaleVector2d(view_->size_allocation().OffsetFromOrigin(),
                       1.f / view_->scale_factor());
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

void View::SchedulePaintRect(const RectF& rect) {
  Rect relative = ToEnclosedRect(ScaleRect(rect, GetNative()->scale_factor()));
  GetNative()->Invalidate(relative +
                          GetNative()->size_allocation().OffsetFromOrigin());
}

void View::PlatformSetVisible(bool visible) {
  GetNative()->SetVisible(visible);
}

bool View::IsVisible() const {
  return GetNative()->is_visible();
}

bool View::IsTreeVisible() const {
  return GetNative()->is_tree_visible();
}

void View::SetEnabled(bool enable) {
  GetNative()->SetState(enable ? ControlState::Normal : ControlState::Disabled);
}

bool View::IsEnabled() const {
  return !GetNative()->is_disabled();
}

void View::Focus() {
  GetNative()->SetFocus(true);
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

void View::SetCapture() {
  if (view_->window())
    view_->window()->SetCapture(GetNative());
}

void View::ReleaseCapture() {
  if (view_->window())
    view_->window()->ReleaseCapture();
}

bool View::HasCapture() const {
  return view_->window() &&
         view_->window()->captured_view() == view_;
}

void View::SetMouseDownCanMoveWindow(bool yes) {
  view_->set_draggable(yes);
}

bool View::IsMouseDownCanMoveWindow() const {
  return view_->is_draggable();
}

int View::DoDragWithOptions(std::vector<Clipboard::Data> data,
                            int operations,
                            const DragOptions& options) {
  if (view_->window())
    return view_->window()->DoDrag(std::move(data), operations, options);
  return DRAG_OPERATION_NONE;
}

void View::CancelDrag() {
  if (view_->window())
    view_->window()->CancelDrag();
}

bool View::IsDragging() const {
  if (view_->window())
    return view_->window()->drag_drop_in_progress();
  return false;
}

void View::RegisterDraggedTypes(std::set<Clipboard::Data::Type> types) {
  view_->RegisterDraggedTypes(std::move(types));
}

void View::PlatformSetCursor(Cursor* cursor) {
}

void View::PlatformSetFont(Font* font) {
  view_->SetFont(font);
}

void View::SetColor(Color color) {
  view_->SetColor(color);
}

void View::SetBackgroundColor(Color color) {
  view_->SetBackgroundColor(color);
}

}  // namespace nu
