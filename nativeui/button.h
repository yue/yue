// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_BUTTON_H_
#define NATIVEUI_BUTTON_H_

#include <string>

#include "nativeui/view.h"

namespace nu {

NATIVEUI_EXPORT class Button : public View {
 public:
  explicit Button(const std::string& title = "");

  void SetTitle(const std::string& title);
  std::string GetTitle() const;

  Signal<void()> on_click;

 protected:
  ~Button() override;
};

}  // namespace nu

#endif  // NATIVEUI_BUTTON_H_
