// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/scroll.h"

#include "nativeui/win/container_win.h"

namespace nu {

namespace {

class ScrollView : public ContainerView,
                   public ContainerView::Delegate {
 public:
  explicit ScrollView(Scroll* delegate)
      : ContainerView(this, ControlType::Scroll),
        delegate_(delegate) {}

  void SetOrigin(const Vector2d& origin) {
    origin_ = origin;
    Layout();
  }

  void SetContentSize(const Size& size) {
    content_size_ = size;
    Layout();
  }

  void OnScroll(int x, int y) {
    Size viewport_size = size_allocation().size();
    Vector2d new_origin = origin_ + Vector2d(x, y);
    if (-new_origin.x() + viewport_size.width() > content_size_.width())
      new_origin.set_x(viewport_size.width() - content_size_.width());
    if (new_origin.x() > 0)
      new_origin.set_x(0);
    if (-new_origin.y() + viewport_size.height() > content_size_.height())
      new_origin.set_y(viewport_size.height() - content_size_.height());
    if (new_origin.y() > 0)
      new_origin.set_y(0);

    if (origin_ != new_origin) {
      origin_ = new_origin;
      Layout();
    }
  }

  // ContainerView::Delegate:
  void Layout() override {
    Rect child_alloc = Rect((size_allocation() + origin_).origin(),
                            content_size_);
    delegate_->GetContentView()->view()->SizeAllocate(child_alloc);
  }

  std::vector<View*> GetChildren() override {
    return std::vector<View*>{delegate_->GetContentView()};
  }

  // BaseView:
  bool OnMouseWheel(bool vertical, UINT flags, int delta,
                    const Point& point) override {
    if (vertical)
      OnScroll(0, delta);
    else
      OnScroll(delta, 0);
    return true;
  }

  void Draw(PainterWin* painter, const Rect& dirty) override {
    painter->Save();
    painter->ClipRect(Rect(size_allocation().size()));
    painter->Translate(origin_);
    delegate_->GetContentView()->view()->Draw(painter, dirty - origin_);
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

void Scroll::SetScrollBarPolicy(Policy h_policy, Policy v_policy) {
}

std::tuple<Scroll::Policy, Scroll::Policy> Scroll::GetScrollBarPolicy() const {
  return std::make_tuple(Policy::Automatic, Policy::Automatic);
}

}  // namespace nu
