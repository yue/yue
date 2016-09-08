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

  void SetPixelBounds(const Rect& bounds) override {
    BaseView::SetPixelBounds(bounds);
    delegate_->GetContentView()->SetPixelBounds(Rect(bounds.size()));
  }

  void Draw(PainterWin* painter, const Rect& dirty) override {
    painter->Save();
    painter->ClipRect(Rect(GetPixelBounds().size()));
    delegate_->GetContentView()->view()->Draw(painter, dirty);
    painter->Restore();
  }

 private:
  Scroll* delegate_;
};

}  // namespace

void Scroll::PlatformInit(const Size& size) {
  TakeOverView(new ScrollView(this));
}

void Scroll::PlatformSetContentView(Container* container) {
  container->view()->SetParent(view());
  container->SetPixelBounds(Rect(GetPixelBounds().size()));
}

void Scroll::SetContentSize(const Size& size) {
  GetContentView()->SetBounds(Rect(size));
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
