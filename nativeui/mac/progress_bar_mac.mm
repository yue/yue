// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/progress_bar.h"

#import <Cocoa/Cocoa.h>

namespace nu {

ProgressBar::ProgressBar() {
  auto* progress = [[NSProgressIndicator alloc] init];
  progress.indeterminate = NO;
  TakeOverView(progress);
  UpdateDefaultStyle();
}

ProgressBar::~ProgressBar() {
}

void ProgressBar::SetValue(float value) {
  auto* progress = static_cast<NSProgressIndicator*>(GetNative());
  progress.indeterminate = NO;
  progress.doubleValue = value;
}

float ProgressBar::GetValue() const {
  auto* progress = static_cast<NSProgressIndicator*>(GetNative());
  return progress.doubleValue;
}

void ProgressBar::SetIndeterminate(bool indeterminate) {
  auto* progress = static_cast<NSProgressIndicator*>(GetNative());
  progress.indeterminate = indeterminate;
  if (indeterminate)
    [progress startAnimation:nil];
  else
    [progress stopAnimation:nil];
}

bool ProgressBar::IsIndeterminate() const {
  auto* progress = static_cast<NSProgressIndicator*>(GetNative());
  return progress.isIndeterminate;
}

SizeF ProgressBar::GetMinimumSize() const {
  auto* progress = static_cast<NSProgressIndicator*>(GetNative());
  return SizeF(0, progress.intrinsicContentSize.height);
}

}  // namespace nu
