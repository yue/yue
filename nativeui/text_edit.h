// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_TEXT_EDIT_H_
#define NATIVEUI_TEXT_EDIT_H_

#include <string>

#include "nativeui/view.h"

namespace nu {

class NATIVEUI_EXPORT TextEdit : public View {
 public:
  TextEdit();

  // View class name.
  static const char kClassName[];

  // View:
  const char* GetClassName() const override;

  std::string GetText() const;
  void Cut();
  void Copy();
  void Paste();
  void Clear();

  // Events.
  Signal<void(TextEdit*)> on_text_change;

 protected:
  ~TextEdit() override;
};

}  // namespace nu

#endif  // NATIVEUI_TEXT_EDIT_H_
