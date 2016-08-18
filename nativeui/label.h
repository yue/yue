// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LABEL_H_
#define NATIVEUI_LABEL_H_

#include <string>

#include "nativeui/view.h"

namespace nu {

NATIVEUI_EXPORT class Label : public View {
 public:
  Label();

  void SetText(const std::string& text);
  std::string GetText();

 protected:
  ~Label() override;
};

}  // namespace nu

#endif  // NATIVEUI_LABEL_H_
