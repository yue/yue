// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/entry.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/screen.h"
#include "nativeui/gfx/win/text_win.h"
#include "nativeui/state.h"
#include "nativeui/win/subwin_view.h"
#include "nativeui/win/util/hwnd_util.h"
#include "nativeui/win/window_win.h"

namespace nu {

namespace {

const int kEntryPadding = 1;

class EntryImpl : public SubwinView {
 public:
  explicit EntryImpl(Entry* delegate)
      : SubwinView(delegate,
                   L"edit",
                   ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE,
                   WS_EX_CLIENTEDGE),
        proc_(SetWindowProc(hwnd(), &WndProc)) {
  }

  bool CanHaveFocus() const override {
    return true;
  }

  void OnCommand(UINT code, int command) override {
    if (code == EN_CHANGE)
      static_cast<Entry*>(delegate())->on_text_change.Emit();
  }

 protected:
  static LRESULT WndProc(HWND hwnd, UINT message, WPARAM w_param,
                         LPARAM l_param) {
    auto* self = reinterpret_cast<EntryImpl*>(GetWindowUserData(hwnd));
    if (!self)  // could happen during destruction
      return 0;
    if (message == WM_CHAR && w_param == VK_RETURN) {
      // Pressing enter means activate.
      static_cast<Entry*>(self->delegate())->on_activate.Emit();
      return 0;
    } else if (message == WM_CHAR && w_param == VK_TAB) {
      // Let the parent handle focus switching.
      ::SendMessage(::GetParent(hwnd), WM_CHAR, w_param, l_param);
      return 0;
    } else if (message == WM_SETFOCUS) {
      // Notify the window that focus has changed.
      if (self->window()) {
        self->window()->focus_manager()->TakeFocus(self->delegate());
      }
    }
    return CallWindowProc(self->proc_, hwnd, message, w_param, l_param);
  }

 private:
  WNDPROC proc_ = nullptr;
};

}  // namespace

Entry::Entry() {
  TakeOverView(new EntryImpl(this));

  Font* font = State::GetCurrent()->GetDefaultFont();
  float height = MeasureText(L"some text", font).height() / GetScaleFactor() +
                 2 * kEntryPadding;
  SetDefaultStyle(SizeF(0, height));
}

Entry::~Entry() {
}

void Entry::SetText(const std::string& text) {
  ::SetWindowTextW(static_cast<SubwinView*>(GetNative())->hwnd(),
                   base::UTF8ToUTF16(text).c_str());
}

std::string Entry::GetText() const {
  return base::UTF16ToUTF8(
      GetWindowString(static_cast<SubwinView*>(GetNative())->hwnd()));
}

}  // namespace nu
