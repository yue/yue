// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/combo_box.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/win/picker_win.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

namespace {

class ComboBoxImpl : public PickerImpl {
 public:
  explicit ComboBoxImpl(ComboBox* delegate)
    : PickerImpl(delegate, CBS_DROPDOWN) {
    // Get the edit window handle to combo box.
    COMBOBOXINFO info = { sizeof(COMBOBOXINFO) };
    if (::GetComboBoxInfo(hwnd(), &info)) {
      HWND edit = info.hwndItem;
      // Subclass the edit control.
      SetWindowUserData(edit, this);
      proc_ = SetWindowProc(edit, &EditWndProc);
    }
  }

 protected:
  // PickerImpl:
  void OnCommand(UINT code, int command) override {
    ComboBox* combobox = static_cast<ComboBox*>(delegate());
    if (code == CBN_EDITCHANGE)
      combobox->on_text_change.Emit(combobox);
    PickerImpl::OnCommand(code, command);
  }

 private:
  static LRESULT CALLBACK EditWndProc(HWND hwnd,
                                      UINT message,
                                      WPARAM w_param,
                                      LPARAM l_param) {
    auto* self = reinterpret_cast<ComboBoxImpl*>(GetWindowUserData(hwnd));
    // Handle the TAB key.
    if (message == WM_CHAR && w_param == VK_TAB && self->window()) {
      self->window()->AdvanceFocus();
      return 0;
    }
    return CallWindowProc(self->proc_, hwnd, message, w_param, l_param);
  }

  WNDPROC proc_;
};

}  // namespace

ComboBox::ComboBox() : Picker(new ComboBoxImpl(this)) {
}

ComboBox::~ComboBox() {
}

void ComboBox::SetText(const std::string& text) {
  auto* combobox = static_cast<ComboBoxImpl*>(GetNative());
  std::wstring text16 = base::UTF8ToWide(text);
  ::SetWindowTextW(combobox->hwnd(), text16.c_str());
}

std::string ComboBox::GetText() const {
  auto* combobox = static_cast<ComboBoxImpl*>(GetNative());
  return base::WideToUTF8(GetWindowString(combobox->hwnd()));
}

void ComboBox::AddItem(const std::string& text) {
  auto* picker = static_cast<PickerImpl*>(GetNative());
  std::wstring text16 = base::UTF8ToWide(text);
  ::SendMessage(picker->hwnd(), CB_ADDSTRING, 0,
                reinterpret_cast<LPARAM>(text16.c_str()));
}

}  // namespace nu
