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
  SetPreferredSize(SizeF(0, height), SizeF(100, height));
}

Progress::~Progress() {
}

void Progress::SetValue(int value) {
  auto* progress = static_cast<NSProgressIndicator*>(view());
  progress.indeterminate = NO;
  progress.doubleValue = value;
}

int Progress::GetValue() const {
  auto* progress = static_cast<NSProgressIndicator*>(view());
  return progress.doubleValue;
}

void Progress::SetIndeterminate(bool indeterminate) {
  auto* progress = static_cast<NSProgressIndicator*>(view());
  progress.indeterminate = indeterminate;
  if (indeterminate)
    [progress startAnimation:nil];
  else
    [progress stopAnimation:nil];
}

bool Progress::IsIndeterminate() const {
  auto* progress = static_cast<NSProgressIndicator*>(view());
  return progress.isIndeterminate;
}

}  // namespace nu
