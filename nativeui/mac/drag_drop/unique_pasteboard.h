// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVEUI_MAC_DRAG_DROP_UNIQUE_PASTEBOARD_H_
#define NATIVEUI_MAC_DRAG_DROP_UNIQUE_PASTEBOARD_H_

#import <AppKit/AppKit.h>

#include "base/mac/scoped_nsobject.h"

namespace nu {

class UniquePasteboard {
 public:
  UniquePasteboard();
  ~UniquePasteboard();

  NSPasteboard* get() { return pasteboard_; }

 private:
  base::scoped_nsobject<NSPasteboard> pasteboard_;
};

}  // namespace nu

#endif  // NATIVEUI_MAC_DRAG_DROP_UNIQUE_PASTEBOARD_H_
