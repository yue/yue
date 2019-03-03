// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/entry.h"

#include "nativeui/gfx/attributed_text.h"
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

Entry::Entry(Type type) {
  auto* edit = new EntryImpl(this);
  TakeOverView(edit);
  UpdateDefaultStyle();

  if (type == Type::Password) {
    WPARAM asterisk = static_cast<WPARAM>('*');
    ::SendMessage(edit->hwnd(), EM_SETPASSWORDCHAR, asterisk, 0L);
  }
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
  scoped_refptr<AttributedText> attributed_text =
      new AttributedText(L"some text", TextFormat());
  attributed_text->SetFont(GetNative()->font());
  return SizeF(0, attributed_text->GetSize().height() + 2 * kEntryPadding);
}

}  // namespace nu
