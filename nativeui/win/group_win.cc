// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/insets.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/geometry/vector2d.h"
#include "nativeui/gfx/win/text_win.h"
#include "nativeui/win/container_win.h"

namespace nu {

namespace {

// The offset of title to left of rect.
const int kTitleLeftMargin = 5;

class GroupView : public ContainerView,
                  public ContainerView::Delegate {
 public:
  explicit GroupView(Group* delegate)
      : ContainerView(this, ControlType::Group),
        delegate_(delegate),
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

  Insets GetBorder() const {
    int padding = std::ceil(2 * scale_factor());
    return Insets(title_bounds_.height() + padding, padding * 2,
                  padding * 2, padding * 2);
  }

  // ContainerView:
  void Layout() override {
    Rect child_alloc(size_allocation());
    child_alloc.Inset(GetBorder());
    delegate_->GetContentView()->view()->SizeAllocate(child_alloc);
  }

  std::vector<BaseView*> GetChildren() override {
    return std::vector<BaseView*>{delegate_->GetContentView()->view()};
  }

  // BaseView:
  void Draw(PainterWin* painter, const Rect& dirty) override {
    // Draw title.
    if (dirty.Intersects(title_bounds_))
      painter->DrawString(title_, font_, color_, title_bounds_);

    // Calculate the border bounds.
    Rect drawing_bounds(size_allocation().size());
    Insets border = GetBorder();
    Rect border_bounds(drawing_bounds);
    border_bounds.Inset(border.left() / 2, border.top() / 2,
                        border.right() / 2, border.bottom() / 2);

    // Draw border.
    painter->Save();
    painter->ClipRect(drawing_bounds);
    painter->ClipRect(title_bounds_, Painter::CombineMode::Exclude);
    painter->DrawRect(border_bounds, Color(255, 170, 170, 170));
    painter->Restore();

    // Draw child.
    Vector2d child_offset(border.left(), border.top());
    painter->Save();
    painter->Translate(child_offset);
    delegate_->GetContentView()->view()->Draw(painter, dirty - child_offset);
    painter->Restore();
  }

 private:
  Group* delegate_;

  Color color_;
  Font font_;
  Rect title_bounds_;

  base::string16 title_;
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

Size Group::GetBorderSize() const {
  GroupView* group = static_cast<GroupView*>(view());
  Rect bounds;
  bounds.Inset(-group->GetBorder());
  return bounds.size();
}

}  // namespace nu
