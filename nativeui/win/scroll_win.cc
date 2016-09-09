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
    Layout();
  }

  void SetContentSize(const Size& size) {
    content_size_ = size;
    Layout();
  }

  void Layout() {
    Rect child_alloc = Rect((size_allocation() + origin_).origin(),
                            content_size_);
    delegate_->GetContentView()->view()->SizeAllocate(child_alloc);
  }

  void Draw(PainterWin* painter, const Rect& dirty) override {
    painter->Save();
    painter->ClipRect(Rect(size_allocation().size()));
    painter->Translate(origin_);
    delegate_->GetContentView()->view()->Draw(painter, dirty + origin_);
    painter->Restore();
  }

 private:
  Size content_size_;
  Vector2d origin_;

  Scroll* delegate_;
};

}  // namespace

void Scroll::PlatformInit(const Size& size) {
  TakeOverView(new ScrollView(this));
}

void Scroll::PlatformSetContentView(Container* container) {
  container->view()->SetParent(view());

  auto* scroll = static_cast<ScrollView*>(view());
  scroll->SetContentSize(container->view()->size_allocation().size());
}

void Scroll::SetContentSize(const Size& size) {
  auto* scroll = static_cast<ScrollView*>(view());
  scroll->SetContentSize(ScaleToCeiledSize(size, scroll->scale_factor()));
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
