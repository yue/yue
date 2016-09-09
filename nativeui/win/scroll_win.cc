// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/scroll.h"

#include "nativeui/win/base_view.h"

namespace nu {

namespace {

class ScrollView : public BaseView {
 public:
  explicit ScrollView(Scroll* delegate)
      : BaseView(ControlType::Scroll), delegate_(delegate) {}

  void SetOrigin(const Vector2d& origin) {
    origin_ = origin;
    Invalidate();
  }

  void Draw(PainterWin* painter, const Rect& dirty) override {
    painter->Save();
    painter->ClipRect(Rect(size_allocation().size()));
    painter->Translate(origin_);
    delegate_->GetContentView()->view()->Draw(painter, dirty + origin_);
    painter->Restore();
  }

  void set_content_pixel_size(const Size& size) { content_size_ = size; }

 private:
  Size content_size_;
  Vector2d origin_;

  Scroll* delegate_;
};

}  // namespace

void Scroll::PlatformInit(const Size& size) {
  TakeOverView(new ScrollView(this));
  // static_cast<ScrollView*>(view())->SetOrigin(Vector2d(-20, -30));
}

void Scroll::PlatformSetContentView(Container* container) {
  container->view()->SetParent(view());

  auto* scroll = static_cast<ScrollView*>(view());
  scroll->set_content_pixel_size(container->view()->size_allocation().size());
  scroll->Invalidate();
}

void Scroll::SetContentSize(const Size& size) {
  auto* scroll = static_cast<ScrollView*>(view());
  scroll->set_content_pixel_size(ScaleToCeiledSize(size,
                                                   scroll->scale_factor()));
  scroll->Invalidate();
}

void Scroll::SetVerticalScrollBar(bool has) {
}

bool Scroll::HasVerticalScrollBar() const {
  return false;
}

void Scroll::SetHorizontalScrollBar(bool has) {
}

bool Scroll::HasHorizontalScrollBar() const {
  return false;
}

void Scroll::SetAutoHideScrollBar(bool is) {
}

bool Scroll::IsScrollBarAutoHide() const {
  return false;
}

}  // namespace nu
