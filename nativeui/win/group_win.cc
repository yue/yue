// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_util.h"
#include "nativeui/win/base_view.h"
#include "ui/gfx/geometry/insets.h"

namespace nu {

namespace {

class GroupView : public BaseView {
 public:
  explicit GroupView(Group* delegate)
      : BaseView(true), delegate_(delegate),
        border_insets_(10, 5, 5, 5) {}

  void SetTitle(const base::string16& title) {
    title_ = title;
  }

  base::string16 GetTitle() const {
    return title_;
  }

  void Layout() {
    gfx::Rect child_bounds(gfx::Rect(GetPixelBounds().size()));
    child_bounds.Inset(border_insets_);
    delegate_->GetContentView()->view()->SetPixelBounds(child_bounds);
  }

  void SetPixelBounds(const gfx::Rect& pixel_bounds) override {
    BaseView::SetPixelBounds(pixel_bounds);
    Layout();
  }

  void Draw(Gdiplus::Graphics* context, const gfx::Rect& dirty) override {
    gfx::Rect child_dirty(GetPixelBounds().size());
    child_dirty.Inset(border_insets_);
    delegate_->GetContentView()->view()->Draw(context, dirty);
  }

  void SetParent(BaseView* parent) override {
    BaseView::SetParent(parent);
    delegate_->GetContentView()->view()->SetParent(this);
  }

 private:
  Group* delegate_;

  gfx::Insets border_insets_;
  base::string16 title_;
};

}  // namespace

void Group::PlatformInit() {
  set_view(new GroupView(this));
}

void Group::PlatformSetContentView(Container* container) {
  container->view()->SetParent(view());
  static_cast<GroupView*>(view())->Layout();
}

void Group::SetTitle(const std::string& title) {
  static_cast<GroupView*>(view())->SetTitle(base::UTF8ToUTF16(title));
}

std::string Group::GetTitle() const {
  return base::UTF16ToUTF8(static_cast<GroupView*>(view())->GetTitle());
}

}  // namespace nu
