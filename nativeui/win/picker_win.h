// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_PICKER_WIN_H_
#define NATIVEUI_WIN_PICKER_WIN_H_

#include <string>

#include "nativeui/picker.h"
#include "nativeui/win/subwin_view.h"

namespace nu {

class PickerImpl : public SubwinView {
 public:
  PickerImpl(Picker* delegate, int style);

  int ItemCount() const;
  std::string GetItemAt(int i);
  void SelectItemAt(int index);
  int GetSelectedItemIndex() const;

 protected:
  // SubwinView:
  void OnCommand(UINT code, int command) override;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_PICKER_WIN_H_
