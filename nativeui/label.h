// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LABEL_H_
#define NATIVEUI_LABEL_H_

#include <string>

#include "nativeui/view.h"

namespace nu {

class NATIVEUI_EXPORT Label : public View {
 public:
  explicit Label(const std::string& text = "");

  // View class name.
  static const char kClassName[];

  void SetText(const std::string& text);
  std::string GetText() const;

  // View:
  const char* GetClassName() const override;
  SizeF GetMinimumSize() const override;

 protected:
  ~Label() override;

 private:
  void PlatformSetText(const std::string& text);
};

}  // namespace nu

#endif  // NATIVEUI_LABEL_H_
