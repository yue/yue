// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/picker_win.h"

#include <commctrl.h>

#include <vector>

#include "base/strings/utf_string_conversions.h"

namespace nu {

PickerImpl::PickerImpl(Picker* delegate, int style)
    : SubwinView(delegate, WC_COMBOBOX,
                 style | WS_CHILD | WS_VISIBLE) {
  set_focusable(true);
}

int PickerImpl::ItemCount() const {
  int count = ::SendMessage(hwnd(), CB_GETCOUNT, 0, 0L);
  return count == CB_ERR ? 0 : count;
}

std::string PickerImpl::GetItemAt(int i) {
  int len = ::SendMessage(hwnd(), CB_GETLBTEXTLEN, i, 0L);
  if (len == CB_ERR)
    return std::string();
  std::wstring text16(len, 0L);
  ::SendMessage(hwnd(), CB_GETLBTEXT, i,
                reinterpret_cast<LPARAM>(text16.data()));
  return base::WideToUTF8(text16);
}

void PickerImpl::SelectItemAt(int index) {
  ::SendMessage(hwnd(), CB_SETCURSEL, index, 0L);
}

int PickerImpl::GetSelectedItemIndex() const {
  return ::SendMessage(hwnd(), CB_GETCURSEL, 0, 0L);
}

void PickerImpl::OnCommand(UINT code, int command) {
  Picker* picker = static_cast<Picker*>(delegate());
  if (code == CBN_SELENDOK)
    picker->on_selection_change.Emit(picker);
}

///////////////////////////////////////////////////////////////////////////////
// Public Picker API implementation.

Picker::Picker() : Picker(new PickerImpl(this, CBS_DROPDOWNLIST)) {
}

Picker::Picker(NativeView view) {
  TakeOverView(view);
  UpdateDefaultStyle();
}

void Picker::AddItem(const std::string& text) {
  auto* picker = static_cast<PickerImpl*>(GetNative());
  std::wstring text16 = base::UTF8ToWide(text);
  // Guard against duplicate items.
  int r = ::SendMessage(picker->hwnd(), CB_FINDSTRINGEXACT, 0,
                        reinterpret_cast<LPARAM>(text16.c_str()));
  if (r == CB_ERR) {
    ::SendMessage(picker->hwnd(), CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(text16.c_str()));
    // Select first item by default.
    if (picker->ItemCount() == 1)
      picker->SelectItemAt(0);
  }
}

void Picker::RemoveItemAt(int index) {
  auto* picker = static_cast<PickerImpl*>(GetNative());
  ::SendMessage(picker->hwnd(), CB_DELETESTRING, index, 0L);
}

std::vector<std::string> Picker::GetItems() const {
  auto* picker = static_cast<PickerImpl*>(GetNative());
  std::vector<std::string> result;
  // There is no single API to return all items, so we just iterate.
  int count = picker->ItemCount();
  result.reserve(count);
  for (int i = 0; i < count; ++i)
    result.emplace_back(picker->GetItemAt(i));
  return result;
}

void Picker::SelectItemAt(int index) {
  auto* picker = static_cast<PickerImpl*>(GetNative());
  picker->SelectItemAt(index);
}

std::string Picker::GetSelectedItem() const {
  auto* picker = static_cast<PickerImpl*>(GetNative());
  return picker->GetItemAt(picker->GetSelectedItemIndex());
}

int Picker::GetSelectedItemIndex() const {
  auto* picker = static_cast<PickerImpl*>(GetNative());
  return picker->GetSelectedItemIndex();
}

SizeF Picker::GetMinimumSize() const {
  return SizeF(10, 20);
}

}  // namespace nu
