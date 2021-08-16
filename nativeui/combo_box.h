// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_COMBO_BOX_H_
#define NATIVEUI_COMBO_BOX_H_

#include <string>
#include <vector>

#include "nativeui/picker.h"

namespace nu {

class NATIVEUI_EXPORT ComboBox : public Picker {
 public:
  ComboBox();

  // View class name.
  static const char kClassName[];

  void SetText(const std::string& text);
  std::string GetText() const;

  // Picker:
  void AddItem(const std::string& text) override;
#if defined(OS_MAC)
  // On macOS the ComboBox does not have any relationship with Picker, we have
  // to reimplement every method.
  void RemoveItemAt(int index) override;
  std::vector<std::string> GetItems() const override;
  void SelectItemAt(int index) override;
  std::string GetSelectedItem() const override;
  int GetSelectedItemIndex() const override;

  // View:
  SizeF GetMinimumSize() const override;
#endif
  const char* GetClassName() const override;

  // Events.
  Signal<void(ComboBox*)> on_text_change;

 protected:
  ~ComboBox() override;
};

}  // namespace nu

#endif  // NATIVEUI_COMBO_BOX_H_
