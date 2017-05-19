// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_BUTTON_H_
#define NATIVEUI_BUTTON_H_

#include <string>

#include "nativeui/view.h"

namespace nu {

class NATIVEUI_EXPORT Button : public View {
 public:
  enum class Type {
    Normal,
    Checkbox,
    Radio,
  };
  explicit Button(const std::string& title, Type type = Type::Normal);

  // View class name.
  static const char kClassName[];

  void SetTitle(const std::string& title);
  std::string GetTitle() const;
  void SetChecked(bool checked);
  bool IsChecked() const;

  // View:
  const char* GetClassName() const override;

  // Events.
  Signal<void(Button*)> on_click;

 protected:
  ~Button() override;
};

}  // namespace nu

#endif  // NATIVEUI_BUTTON_H_
