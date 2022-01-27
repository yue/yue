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

  // Note: After adding any new method, make sure it is also implemented in
  // ComboBox on macOS.
  virtual void AddItem(const std::string& text);
  virtual void RemoveItemAt(int index);
  virtual std::vector<std::string> GetItems() const;
  virtual void SelectItemAt(int index);
  virtual std::string GetSelectedItem() const;
  virtual int GetSelectedItemIndex() const;

  // View:
  const char* GetClassName() const override;
  SizeF GetMinimumSize() const override;

  // Events.
  Signal<void(Picker*)> on_selection_change;

 protected:
  // Used by subclass.
  explicit Picker(NativeView view);

  ~Picker() override;
};

}  // namespace nu

#endif  // NATIVEUI_PICKER_H_
