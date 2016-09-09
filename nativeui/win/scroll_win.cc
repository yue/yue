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
    content_view()->SizeAllocate(child_alloc);
  }

  void SizeAllocate(const Rect& size_allocation) override {
    BaseView::SizeAllocate(size_allocation);
    Layout();
  }

  void OnMouseMove(UINT flags, const Point& point) override {
    if (content_view()->size_allocation().Contains(point)) {
      if (!content_view_hovered_) {
        content_view_hovered_ = true;
        content_view()->OnMouseEnter();
      }
      content_view()->OnMouseMove(flags, point);
    } else if (content_view_hovered_) {
      OnMouseLeave();
    }
  }

  void OnMouseLeave() override {
    if (content_view_hovered_) {
      content_view_hovered_ = false;
      content_view()->OnMouseLeave();
    }
  }

  void OnMouseClick(UINT message, UINT flags, const Point& point) override {
    if (content_view()->size_allocation().Contains(point))
      content_view()->OnMouseClick(message, flags, point);
  }

  void Draw(PainterWin* painter, const Rect& dirty) override {
    painter->Save();
    painter->ClipRect(Rect(size_allocation().size()));
    painter->Translate(origin_);
    delegate_->GetContentView()->view()->Draw(painter, dirty + origin_);
    painter->Restore();
  }

  void SetParent(BaseView* parent) override {
    BaseView::SetParent(parent);
    delegate_->GetContentView()->view()->SetParent(this);
  }

  BaseView* content_view() const { return delegate_->GetContentView()->view(); }

 private:
  Size content_size_;
  Vector2d origin_;

  Scroll* delegate_;

  // Whether the content view is hovered.
  bool content_view_hovered_ = false;
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

void Scroll::SetScrollBarPolicy(Policy h_policy, Policy v_policy) {
}

std::tuple<Scroll::Policy, Scroll::Policy> Scroll::GetScrollBarPolicy() const {
  return std::make_tuple(Policy::Automatic, Policy::Automatic);
}

}  // namespace nu
