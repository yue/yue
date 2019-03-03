// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/group.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/attributed_text.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/geometry/insets.h"
#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/gfx/geometry/vector2d.h"
#include "nativeui/win/container_win.h"

namespace nu {

namespace {

// The offset of title to left of rect.
const float kTitleLeftMargin = 5.f;

// The padding between title and border.
const float kTitlePadding = 2.f;

// The padding between border and content.
const float kPadding = 2.f;

class GroupImpl : public ContainerImpl,
                  public ContainerImpl::Adapter {
 public:
  explicit GroupImpl(Group* delegate)
      : ContainerImpl(delegate, this), delegate_(delegate) {}

  void SetTitle(const base::string16& title) {
    text_ = new AttributedText(
        title, {TextAlign::Center, TextAlign::Start, false /* wrap */});
    text_->SetColor(color());
    UpdateTitleBounds();
  }

  InsetsF GetBorder() const {
    return Insets(title_bounds_.height() + kPadding, kPadding * 2,
                  kPadding * 2, kPadding * 2);
  }

  // ContainerImpl:
  void Layout() override {
    RectF child_alloc(delegate_->GetBounds().size());
    child_alloc.Inset(GetBorder());
    delegate_->GetContentView()->SetBounds(child_alloc);
  }

  void ForEach(const std::function<bool(ViewImpl*)>& callback,
               bool reverse) override {
    callback(delegate_->GetContentView()->GetNative());
  }

  bool HasChild(ViewImpl* child) override {
    return child == delegate_->GetContentView()->GetNative();
  }

  // ViewImpl:
  void Draw(PainterWin* painter, const Rect& raw_dirty) override {
    // Draw title.
    RectF dirty = ScaleRect(RectF(raw_dirty), 1.f / scale_factor());
    if (dirty.Intersects(title_bounds_))
      painter->DrawAttributedText(text_.get(), title_bounds_);

    // Bounds of the content view.
    RectF child_bounds(delegate_->GetBounds().size());
    InsetsF border = GetBorder();
    child_bounds.Inset(border);

    // Draw border.
    painter->BeginPath();
    painter->MoveTo(PointF(title_bounds_.x(),
                           child_bounds.y() - border.top() / 2.0f));
    painter->LineTo(PointF(child_bounds.x() - border.left() / 2.0f,
                           child_bounds.y()  - border.top() / 2.0f));
    painter->LineTo(PointF(child_bounds.x()   - border.left() / 2.0f,
                           child_bounds.bottom() + border.bottom() / 2.f));
    painter->LineTo(PointF(child_bounds.right()  + border.right() / 2,
                           child_bounds.bottom() + border.bottom() / 2.f));
    painter->LineTo(PointF(child_bounds.right() + border.right() / 2.f,
                           child_bounds.y()   - border.top() / 2.f));
    painter->LineTo(PointF(title_bounds_.right(),
                           child_bounds.y()   - border.top() / 2.f));
    painter->SetColor(Color(255, 170, 170, 170));
    painter->Stroke();

    // Draw child.
    child_bounds.Intersect(dirty);
    if (child_bounds.IsEmpty())
      return;
    Vector2d child_offset(border.left(), border.top());
    painter->Save();
    painter->ClipRect(child_bounds);
    painter->Translate(child_offset);
    delegate_->GetContentView()->GetNative()->Draw(
        painter,
        ToEnclosedRect(ScaleRect(child_bounds - child_offset, scale_factor())));
    painter->Restore();
  }

  void SetFont(Font* font) override {
    ViewImpl::SetFont(font);
    UpdateTitleBounds();
  }

  void SetColor(Color color) override {
    ViewImpl::SetColor(color);
    text_->SetColor(color);
  }

  AttributedText* text() const { return text_.get(); }

 private:
  void UpdateTitleBounds() {
    text_->SetFont(font());
    SizeF size = text_->GetSize();
    title_bounds_.SetRect(kTitleLeftMargin, 0,
                          size.width() + 2 * kTitlePadding, size.height());
  }

  Group* delegate_;
  RectF title_bounds_;
  scoped_refptr<AttributedText> text_;
};

}  // namespace

void Group::PlatformInit() {
  TakeOverView(new GroupImpl(this));
}

void Group::PlatformSetContentView(View* view) {
  view->GetNative()->SetParent(GetNative());
  static_cast<GroupImpl*>(GetNative())->Layout();
}

void Group::SetTitle(const std::string& title) {
  static_cast<GroupImpl*>(GetNative())->SetTitle(base::UTF8ToUTF16(title));
  GetNative()->Invalidate();
}

std::string Group::GetTitle() const {
  GroupImpl* group = static_cast<GroupImpl*>(GetNative());
  return group->text()->GetText();
}

SizeF Group::GetBorderSize() const {
  GroupImpl* group = static_cast<GroupImpl*>(GetNative());
  RectF bounds;
  bounds.Inset(-group->GetBorder());
  return bounds.size();
}

}  // namespace nu
