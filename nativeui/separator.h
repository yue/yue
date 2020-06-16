// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_SEPARATOR_H_
#define NATIVEUI_SEPARATOR_H_

#include "nativeui/standard_enums.h"
#include "nativeui/view.h"

namespace nu {

class NATIVEUI_EXPORT Separator : public View {
 public:
  explicit Separator(Orientation orientation);

  // View class name.
  static const char kClassName[];

  // The height of line.
#if defined(OS_LINUX)
  static const int kLineHeight = 2;
#else
  static const int kLineHeight = 1;
#endif

  // View:
  const char* GetClassName() const override;
  SizeF GetMinimumSize() const override;

 protected:
  ~Separator() override;

 private:
  NativeView PlatformCreate(Orientation orientation);

  Orientation orientation_;
};

}  // namespace nu

#endif  // NATIVEUI_SEPARATOR_H_
