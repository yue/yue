// Copyright 2019 Cheng Zhao. All rights reserved.
// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#include "nativeui/mac/drag_drop/unique_pasteboard.h"

#include "base/mac/mac_util.h"

namespace nu {

UniquePasteboard::UniquePasteboard()
    : pasteboard_([[NSPasteboard pasteboardWithUniqueName] retain]) {}

UniquePasteboard::~UniquePasteboard() {
  [pasteboard_ releaseGlobally];
}

}  // namespace nu
