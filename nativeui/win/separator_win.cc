// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/separator.h"

#include "nativeui/win/subwin_view.h"

namespace nu {

namespace {

class SeparatorImpl : public SubwinView {
 public:
  explicit SeparatorImpl(Separator* delegate)
      : SubwinView(delegate, L"static",
                   SS_ETCHEDHORZ | WS_CHILD | WS_VISIBLE) {}
};

}  // namespace

NativeView Separator::PlatformCreate(Orientation orientation) {
  return new SeparatorImpl(this);
}

}  // namespace nu
