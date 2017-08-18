// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/entry.h"

#include "base/win/scoped_hdc.h"
#include "nativeui/gfx/screen.h"
#include "nativeui/gfx/win/text_win.h"
#include "nativeui/win/edit_view.h"

namespace nu {

namespace {

const int kEntryPadding = 1;

class EntryImpl : public EditView {
 public:
  explicit EntryImpl(Entry* delegate)
      : EditView(delegate, ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE) {
    SetPlainText();
  }

  SizeF GetPreferredSize() const {
    base::win::ScopedGetDC dc(window() ? window()->hwnd() : NULL);
    SizeF size = MeasureText(dc, L"some text", font());
    float height = size.height() / scale_factor() + 2 * kEntryPadding;
    return SizeF(0, height);
  }

  // SubwinView:
  void OnCommand(UINT code, int command) override {
    Entry* entry = static_cast<Entry*>(delegate());
    if (code == EN_CHANGE)
      entry->on_text_change.Emit(entry);
  }

 protected:
  CR_BEGIN_MSG_MAP_EX(EntryImpl, SubwinView)
    CR_MSG_WM_CHAR(OnChar)
  CR_END_MSG_MAP()

  void OnChar(UINT ch, UINT repeat, UINT flags) {
    Entry* entry = static_cast<Entry*>(delegate());
    if (ch == VK_RETURN)  // enter means activate.
      entry->on_activate.Emit(entry);
    else
      SetMsgHandled(false);
  }
};

}  // namespace

Entry::Entry() {
  auto* edit = new EntryImpl(this);
  TakeOverView(edit);
  UpdateDefaultStyle();
}

Entry::~Entry() {
}

void Entry::SetText(const std::string& text) {
  static_cast<EditView*>(GetNative())->SetText(text);
}

std::string Entry::GetText() const {
  return static_cast<EditView*>(GetNative())->GetText();
}

SizeF Entry::GetMinimumSize() const {
  return static_cast<EditView*>(GetNative())->GetPreferredSize();
}

}  // namespace nu
