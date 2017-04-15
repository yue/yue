// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/entry.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/screen.h"
#include "nativeui/gfx/win/text_win.h"
#include "nativeui/state.h"
#include "nativeui/win/subwin_view.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

namespace {

const int kEntryPadding = 1;

class EntryImpl : public SubwinView {
 public:
  explicit EntryImpl(Entry* delegate)
      : SubwinView(delegate,
                   L"edit",
                   ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE,
                   WS_EX_CLIENTEDGE) {
  }

  bool CanHaveFocus() const override {
    return true;
  }

  void OnCommand(UINT code, int command) override {
    if (code == EN_CHANGE)
      static_cast<Entry*>(delegate())->on_text_change.Emit();
  }

 protected:
  CR_BEGIN_MSG_MAP_EX(EntryImpl, SubwinView)
    CR_MSG_WM_CHAR(OnChar)
  CR_END_MSG_MAP()

  void OnChar(UINT ch, UINT repeat, UINT flags) {
    if (ch == VK_RETURN)  // enter means activate.
      static_cast<Entry*>(delegate())->on_activate.Emit();
    else
      SetMsgHandled(false);
  }
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
