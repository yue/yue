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
  enum class Type {
    Normal,
    Password,
  };

  explicit Entry(Type type = Type::Normal);

  // View class name.
  static const char kClassName[];

  void SetText(const std::string& text);
  std::string GetText() const;

  // View:
  const char* GetClassName() const override;
  SizeF GetMinimumSize() const override;

  // Events.
  Signal<void(Entry*)> on_text_change;
  Signal<void(Entry*)> on_activate;

 protected:
  ~Entry() override;
};

}  // namespace nu

#endif  // NATIVEUI_ENTRY_H_
