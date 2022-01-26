// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/popover.h"

#include <utility>

#include "nativeui/gfx/painter.h"
#include "nativeui/screen.h"
#include "nativeui/window.h"

namespace nu {

namespace {

const float kPopoverTriangleHeight = 12;
const float kPopoverPadding = 10;

}  // namespace

class PopoverRootView : public Container {
 public:
  PopoverRootView();

  void SetContentView(scoped_refptr<View> view);
  View* GetContentView() const { return content_view_; }

 protected:
  ~PopoverRootView() override = default;

  // Container:
  void UpdateChildBounds() override;

 private:
  void OnDraw(Container* self, Painter* painter, RectF dirty) {
    float degrees = M_PI / 180;
    float radius = 6;
    float triangle = kPopoverTriangleHeight;
    RectF r(GetBounds().size());

    float y = r.y() + triangle;
    float height = r.height() - triangle;
    painter->BeginPath();
    painter->Arc(PointF(r.x() + r.width() - radius, y + radius),
                 radius, -90 * degrees, 0);
    painter->Arc(PointF(r.x() + r.width() - radius, y + height - radius),
                 radius, 0, 90 * degrees);
    painter->Arc(PointF(r.x() + radius, y + height - radius),
                 radius, 90 * degrees, 180 * degrees);
    painter->Arc(PointF(r.x() + radius, y + radius),
                 radius, 180 * degrees, 270 * degrees);
    painter->LineTo(PointF(r.x() + r.width() / 2 - triangle, y));
    painter->LineTo(PointF(r.x() + r.width() / 2, r.y()));
    painter->LineTo(PointF(r.x() + r.width() / 2 + triangle, y));
    painter->ClosePath();

    painter->SetLineWidth(1);
    painter->SetFillColor(Color::Get(Color::Name::WindowBackground));
    painter->SetStrokeColor(Color::Get(Color::Name::Border));
    painter->DrawPath();
  }

  View* content_view_ = nullptr;
};

PopoverRootView::PopoverRootView() {
  on_draw.Connect(std::bind(&PopoverRootView::OnDraw, this,
                            std::placeholders::_1,
                            std::placeholders::_2,
                            std::placeholders::_3));
}

void PopoverRootView::SetContentView(scoped_refptr<View> view) {
  CHECK(view) << "Content view can not be null";
  if (content_view_)
    RemoveChildView(content_view_);
  content_view_ = view.get();
  AddChildView(std::move(view));
}

void PopoverRootView::UpdateChildBounds() {
  if (content_view_) {
    SizeF content_size = GetBounds().size();
    content_size.Enlarge(-kPopoverPadding * 2,
                         -(kPopoverPadding * 2 + kPopoverTriangleHeight));
    content_view_->SetBounds(RectF(
        PointF(kPopoverPadding, kPopoverPadding + kPopoverTriangleHeight),
        content_size));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public Popover API implementation.

Popover::Popover() {
  Window::Options options;
  options.frame = false;
  options.transparent = true;
  options.no_activate = true;
  window_ = new Window(options);
  window_->SetResizable(false);
  window_->SetSkipTaskbar(true);
  window_->should_close = [](Window*) { return false; };
  window_->on_mouse_up.Connect([this](Window*, const MouseEvent&) {
    PointF pos = Screen::GetCurrent()->GetCursorScreenPoint();
    if (!window_->GetBounds().Contains(pos))
      Close();
    return true;
  });
  window_->on_capture_lost.Connect([this](Window*) {
    Close();
  });

  root_view_ = new PopoverRootView();
  window_->SetContentView(root_view_.get());
}

Popover::~Popover() = default;

void Popover::ShowRelativeTo(View* view) {
  if (!view->GetWindow())
    return;
  RectF vbounds = view->GetBoundsInWindow() +
                  view->GetWindow()->GetContentBounds().OffsetFromOrigin();
  RectF bounds(window_->GetBounds().size());
  bounds.set_x(vbounds.x() + (vbounds.width() - bounds.width()) / 2);
  bounds.set_y(vbounds.bottom() + 1);
  window_->SetBounds(bounds);
  window_->Activate();
  window_->SetCapture();
}

void Popover::Close() {
  if (!window_->IsVisible())
    return;
  window_->ReleaseCapture();
  window_->SetVisible(false);
  on_close.Emit(this);
}

void Popover::SetContentView(scoped_refptr<View> view) {
  root_view_->SetContentView(std::move(view));
}

View* Popover::GetContentView() const {
  return root_view_->GetContentView();
}

void Popover::SetContentSize(const SizeF& size) {
  SizeF root_size = size;
  root_size.Enlarge(kPopoverPadding * 2,
                    kPopoverPadding * 2 + kPopoverTriangleHeight);
  window_->SetContentSize(root_size);
}

}  // namespace nu
