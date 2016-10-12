// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/entry.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/win/text_win.h"
#include "nativeui/state.h"
#include "nativeui/win/subwin_view.h"
#include "nativeui/win/util/hwnd_util.h"
#include "nativeui/win/window_win.h"

namespace nu {

namespace {

const int kEntryPadding = 1;

class EntryView : public SubwinView {
 public:
  explicit EntryView(Entry* delegate)
      : SubwinView(L"edit",
                   ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE,
                   WS_EX_CLIENTEDGE),
        delegate_(delegate),
        proc_(SetWindowProc(hwnd(), &WndProc)) {
  }

  bool CanHaveFocus() const override {
    return true;
  }

  void SetBackgroundColor(Color color) override {
    bg_brush_.reset(CreateSolidBrush(color.ToCOLORREF()));
    BaseView::SetBackgroundColor(color);
  }

  void OnCommand(UINT code, int command) override {
    if (code == EN_CHANGE)
      delegate_->on_text_change.Emit();
  }

  bool OnCtlColor(HDC dc, HBRUSH* brush) override {
    if (!bg_brush_.get())
      return false;
    SetBkMode(dc, TRANSPARENT);
    *brush = bg_brush_.get();
    return true;
  }

 protected:
  static LRESULT WndProc(HWND hwnd, UINT message, WPARAM w_param,
                         LPARAM l_param) {
    auto* self = reinterpret_cast<EntryView*>(GetWindowUserData(hwnd));
    if (message == WM_CHAR && w_param == VK_RETURN) {
      // Pressing enter means activate.
      self->delegate_->on_activate.Emit();
      return 0;
    } else if (message == WM_CHAR && w_param == VK_TAB) {
      // Let the parent handle focus switching.
      ::SendMessage(::GetParent(hwnd), WM_CHAR, w_param, l_param);
      return 0;
    } else if (message == WM_SETFOCUS) {
      // Notify the window that focus has changed.
      if (self->window()) {
        static_cast<TopLevelWindow*>(self->window())->focus_manager()->
          TakeFocus(self->delegate_);
      }
    }
    return CallWindowProc(self->proc_, hwnd, message, w_param, l_param);
  }

 private:
  Entry* delegate_;

  base::win::ScopedGDIObject<HBRUSH> bg_brush_;

  WNDPROC proc_ = nullptr;
};

}  // namespace


Entry::Entry() {
  TakeOverView(new EntryView(this));

  Font* font = State::current()->GetDefaultFont();
  float height = MeasureText(view(), font, L"some text").height() +
                 2 * kEntryPadding * view()->scale_factor();
  SetDefaultStyle(ScaleSize(SizeF(0, height), 1.0f / view()->scale_factor()));
}

Entry::~Entry() {
}

void Entry::SetText(const std::string& text) {
  ::SetWindowTextW(static_cast<SubwinView*>(view())->hwnd(),
                   base::UTF8ToUTF16(text).c_str());
}

std::string Entry::GetText() const {
  return base::UTF16ToUTF8(
      GetWindowString(static_cast<SubwinView*>(view())->hwnd()));
}

}  // namespace nu
