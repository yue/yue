// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/progress.h"

#import <Cocoa/Cocoa.h>

namespace nu {

Progress::Progress() {
  auto* progress = [[NSProgressIndicator alloc] init];
  progress.indeterminate = NO;
  TakeOverView(progress);

  float height = progress.intrinsicContentSize.height;
  SetDefaultStyle(SizeF(0, height));
}

Progress::~Progress() {
}

void Progress::SetValue(int value) {
  auto* progress = static_cast<NSProgressIndicator*>(GetNative());
  progress.indeterminate = NO;
  progress.doubleValue = value;
}

int Progress::GetValue() const {
  auto* progress = static_cast<NSProgressIndicator*>(GetNative());
  return progress.doubleValue;
}

void Progress::SetIndeterminate(bool indeterminate) {
  auto* progress = static_cast<NSProgressIndicator*>(GetNative());
  progress.indeterminate = indeterminate;
  if (indeterminate)
    [progress startAnimation:nil];
  else
    [progress stopAnimation:nil];
}

bool Progress::IsIndeterminate() const {
  auto* progress = static_cast<NSProgressIndicator*>(GetNative());
  return progress.isIndeterminate;
}

}  // namespace nu
