// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/text.h"
#include "nativeui/win/base_view.h"
#include "nativeui/gfx/geometry/insets.h"
#include "nativeui/gfx/geometry/size_conversions.h"

namespace nu {

namespace {

// The offset of title to left of rect.
const int kTitleLeftMargin = 30;

class GroupView : public BaseView {
 public:
  explicit GroupView(Group* delegate)
      : BaseView(true), delegate_(delegate),
        color_(GetThemeColor(ThemeColor::Text)),
        border_insets_(kTitleLeftMargin) {}


  void SetTitle(const base::string16& title) {
    title_ = title;

    // Update the rect of the title.
    title_bounds_ = Rect(Point(kTitleLeftMargin * scale_factor(), 0),
                              ToFlooredSize(MeasureText(font_, title_)));
    border_insets_ = Insets(title_bounds_.height());
  }

  base::string16 GetTitle() const {
    return title_;
  }

  void Layout() {
    Rect child_bounds(Rect(GetPixelBounds().size()));
    child_bounds.Inset(border_insets_);
    delegate_->GetContentView()->view()->SetPixelBounds(child_bounds);
  }

  void SetPixelBounds(const Rect& pixel_bounds) override {
    BaseView::SetPixelBounds(pixel_bounds);
    Layout();
  }

  void Draw(Gdiplus::Graphics* context, const Rect& dirty) override {
    // Draw title.
    Rect title_bounds = title_bounds_ +
                             GetWindowPixelOrigin().OffsetFromOrigin();
    Gdiplus::SolidBrush brush(ToGdi(color_));
    context->DrawString(title_.c_str(), static_cast<int>(title_.size()),
                        font_.GetNativeFont(), ToGdi(title_bounds.origin()),
                        &brush);

    // Calculate the border bounds.
    int text_height = title_bounds.height();
    Rect border_bounds(text_height / 2, text_height / 2,
                            GetPixelBounds().width() - text_height,
                            GetPixelBounds().height() - text_height);
    border_bounds += GetWindowPixelOrigin().OffsetFromOrigin();

    // Draw border.
    Gdiplus::Region region(ToGdi(GetWindowPixelBounds()));
    region.Exclude(ToGdi(title_bounds));
    Gdiplus::GraphicsContainer container = context->BeginContainer();
    context->SetClip(&region);
    Gdiplus::Pen pen(ToGdi(color_));
    context->DrawRectangle(&pen, ToGdi(border_bounds));
    context->EndContainer(container);

    // Draw child.
    Rect child_dirty(GetPixelBounds().size());
    child_dirty.Inset(border_insets_);
    delegate_->GetContentView()->view()->Draw(context, dirty);
  }

  void SetParent(BaseView* parent) override {
    BaseView::SetParent(parent);
    delegate_->GetContentView()->view()->SetParent(this);
  }

 private:
  Group* delegate_;

  Color color_;
  Font font_;
  Rect title_bounds_;

  Insets border_insets_;
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
  view()->Invalidate();
}

std::string Group::GetTitle() const {
  return base::UTF16ToUTF8(static_cast<GroupView*>(view())->GetTitle());
}

}  // namespace nu
