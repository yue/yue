// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_PROGRESS_BAR_H_
#define NATIVEUI_PROGRESS_BAR_H_

#include "nativeui/view.h"

namespace nu {

class NATIVEUI_EXPORT ProgressBar : public View {
 public:
  ProgressBar();

  // View class name.
  static const char kClassName[];

  void SetValue(float value);
  float GetValue() const;

  void SetIndeterminate(bool indeterminate);
  bool IsIndeterminate() const;

  // View:
  const char* GetClassName() const override;
  SizeF GetMinimumSize() const override;

 protected:
  ~ProgressBar() override;
};

}  // namespace nu

#endif  // NATIVEUI_PROGRESS_BAR_H_
