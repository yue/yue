// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_PICKER_H_
#define NATIVEUI_PICKER_H_

#include <string>
#include <vector>

#include "nativeui/view.h"

namespace nu {

class NATIVEUI_EXPORT Picker : public View {
 public:
  Picker();

  // View class name.
  static const char kClassName[];

  void AddItem(const std::string& text);
  void RemoveItemAt(int index);
  std::vector<std::string> GetItems() const;
  void SelectItemAt(int index);
  std::string GetSelectedItem() const;
  int GetSelectedItemIndex() const;

  // View:
  const char* GetClassName() const override;
  SizeF GetMinimumSize() const override;

  // Events.
  Signal<void(Picker*)> on_change;

 protected:
  ~Picker() override;
};

}  // namespace nu

#endif  // NATIVEUI_PICKER_H_
