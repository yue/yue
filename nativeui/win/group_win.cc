// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/win/text_win.h"
#include "nativeui/win/base_view.h"

namespace nu {

namespace {

// The offset of title to left of rect.
const int kTitleLeftMargin = 5;

class GroupView : public BaseView {
 public:
  explicit GroupView(Group* delegate)
      : BaseView(true), delegate_(delegate),
        color_(GetThemeColor(ThemeColor::Text)) {}


  void SetTitle(const base::string16& title) {
    title_ = title;

    // Update the rect of the title.
    title_bounds_ = Rect(Point(kTitleLeftMargin * scale_factor(), 0),
                               ToCeiledSize(MeasureText(this, font_, title_)));
  }

  base::string16 GetTitle() const {
    return title_;
  }

  void Layout() {
    Rect child_bounds(Rect(GetPixelBounds().size()));
    int padding = CalculatePadding();
    child_bounds.Inset(padding * 2, title_bounds_.height() + padding,
                       padding * 2, padding * 2);
    delegate_->GetContentView()->view()->SetPixelBounds(child_bounds);
  }

  void SetPixelBounds(const Rect& pixel_bounds) override {
    BaseView::SetPixelBounds(pixel_bounds);
    Layout();
  }

  void OnMouseMove(const Point& point) override {
    Rect child_bounds = delegate_->GetContentView()->GetPixelBounds();
    if (child_bounds.Contains(point)) {
      if (!content_view_hovered_) {
        content_view_hovered_ = true;
        delegate_->GetContentView()->view()->OnMouseEnter();
      }
      Point child_point = point;
      child_point -= child_bounds.OffsetFromOrigin();
      delegate_->GetContentView()->view()->OnMouseMove(child_point);
    } else if (content_view_hovered_) {
      OnMouseLeave();
    }
  }

  void OnMouseLeave() override {
    if (content_view_hovered_) {
      content_view_hovered_ = false;
      delegate_->GetContentView()->view()->OnMouseLeave();
    }
  }

  void Draw(Gdiplus::Graphics* context, const Rect& dirty) override {
    // Draw title.
    Rect title_bounds =
        title_bounds_ + GetWindowPixelOrigin().OffsetFromOrigin();
    Gdiplus::SolidBrush brush(ToGdi(color_));
    context->DrawString(title_.c_str(), static_cast<int>(title_.size()),
                        font_.GetNativeFont(), ToGdi(title_bounds.origin()),
                        &brush);

    // Calculate the border bounds.
    int text_height = title_bounds.height();
    int padding = CalculatePadding();
    Rect border_bounds(padding, text_height / 2,
                       GetPixelBounds().width() - 2 * padding,
                       GetPixelBounds().height() - text_height / 2 - padding);
    border_bounds += GetWindowPixelOrigin().OffsetFromOrigin();

    // Draw border.
    Gdiplus::Region region(ToGdi(GetWindowPixelBounds()));
    region.Exclude(ToGdi(title_bounds));
    Gdiplus::GraphicsContainer container = context->BeginContainer();
    context->SetClip(&region);
    Gdiplus::Pen pen(Gdiplus::Color(255, 170, 170, 170));
    context->DrawRectangle(&pen, ToGdi(border_bounds));
    context->EndContainer(container);

    // Draw child.
    Rect child_dirty(GetPixelBounds().size());
    child_dirty.Inset(title_bounds_.height() + 1, 3, 3, 3);
    delegate_->GetContentView()->view()->Draw(context, dirty);
  }

  void SetParent(BaseView* parent) override {
    BaseView::SetParent(parent);
    delegate_->GetContentView()->view()->SetParent(this);
  }

  int CalculatePadding() const {
    return std::ceil(2 * scale_factor());
  }

  int text_height() const { return title_bounds_.height(); }

 private:
  Group* delegate_;

  Color color_;
  Font font_;
  Rect title_bounds_;

  base::string16 title_;

  // Whether the content view is hovered.
  bool content_view_hovered_ = false;
};

}  // namespace

void Group::PlatformInit() {
  TakeOverView(new GroupView(this));
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

Size Group::GetBorderPixelSize() const {
  GroupView* group = static_cast<GroupView*>(view());
  int text_height = group->text_height();
  int padding = group->CalculatePadding();
  return Size(padding * 4, text_height + padding * 3);
}

}  // namespace nu
