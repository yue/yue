// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_ENTRY_H_
#define NATIVEUI_ENTRY_H_

#include <string>

#include "nativeui/view.h"

namespace nu {

class NATIVEUI_EXPORT Entry : public View {
 public:
  Entry();

  // View class name.
  static const char kClassName[];

  void SetText(const std::string& text);
  std::string GetText() const;

  // View:
  const char* GetClassName() const override;

  // Events.
  Signal<void()> on_text_change;
  Signal<void()> on_activate;

 protected:
  ~Entry() override;
};

}  // namespace nu

#endif  // NATIVEUI_ENTRY_H_
