// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/progress.h"

#include <commctrl.h>

#include "nativeui/gfx/screen.h"
#include "nativeui/win/subwin_view.h"

namespace nu {

namespace {

class ProgressImpl : public SubwinView {
 public:
  explicit ProgressImpl(Progress* delegate)
      : SubwinView(delegate, PROGRESS_CLASS,
                   PBS_SMOOTH | WS_CHILD | WS_VISIBLE) {}
};

}  // namespace

Progress::Progress() {
  TakeOverView(new ProgressImpl(this));
  SetDefaultStyle(ScaleSize(SizeF(0, GetSystemMetrics(SM_CYVSCROLL)),
                            1.0f / GetScaleFactor()));
}

Progress::~Progress() {
}

void Progress::SetValue(float value) {
  auto* progress = static_cast<ProgressImpl*>(GetNative());
  SetIndeterminate(false);
  SendMessageW(progress->hwnd(), PBM_SETPOS, value, 0);
}

float Progress::GetValue() const {
  return 0;
}

void Progress::SetIndeterminate(bool indeterminate) {
  auto* progress = static_cast<ProgressImpl*>(GetNative());
  DWORD style = GetWindowLong(progress->hwnd(), GWL_STYLE);
  if (indeterminate) {
    SetWindowLong(progress->hwnd(), GWL_STYLE, style | PBS_MARQUEE);
    SendMessageW(progress->hwnd(), PBM_SETMARQUEE, TRUE, 0);
  } else {
    SendMessageW(progress->hwnd(), PBM_SETMARQUEE, FALSE, 0);
    SetWindowLong(progress->hwnd(), GWL_STYLE, style & ~PBS_MARQUEE);
  }
}

bool Progress::IsIndeterminate() const {
  auto* progress = static_cast<ProgressImpl*>(GetNative());
  return (GetWindowLong(progress->hwnd(), GWL_STYLE) & PBS_MARQUEE) != 0;
}

}  // namespace nu
