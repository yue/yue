// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/entry.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/win/text_win.h"
#include "nativeui/win/subwin_view.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

namespace {

const int kEntryPadding = 2;
const int kEntryWidth = 100;

class EntryView : public SubwinView {
 public:
  explicit EntryView(Entry* delegate)
      : SubwinView(L"edit",
                   ES_AUTOHSCROLL | ES_NOHIDESEL | WS_CHILD | WS_VISIBLE |
                   WS_TABSTOP,
                   WS_EX_CLIENTEDGE),
        delegate_(delegate) {
  }

  void OnCommand(UINT code, int command) override {
    if (code == EN_CHANGE)
      delegate_->on_text_change.Emit();
  }

 private:
  Entry* delegate_;
};

}  // namespace


Entry::Entry() {
  TakeOverView(new EntryView(this));

  // Windows doesn't have preferred size for entry, so just add some padding.
  Size text_size = ToCeiledSize(MeasureText(view(), Font(), L"some text"));
  int height = text_size.height() + DIPToPixel(kEntryPadding);
  SetPixelPreferredSize(Size(DIPToPixel(kEntryWidth), height));
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
