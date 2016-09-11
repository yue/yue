// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_PROGRESS_H_
#define NATIVEUI_PROGRESS_H_

#include "nativeui/view.h"

namespace nu {

NATIVEUI_EXPORT class Progress : public View {
 public:
  Progress();

  void SetValue(int value);
  int GetValue() const;

  void SetIndeterminate(bool inditerminate);
  bool IsIndeterminate() const;

 protected:
  ~Progress() override;
};

}  // namespace nu

#endif  // NATIVEUI_PROGRESS_H_
