// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/dragging_info.h"

namespace nu {

DraggingInfo::DraggingInfo(int drag_operation)
    : drag_operation_(drag_operation), weak_factory_(this) {}

DraggingInfo::~DraggingInfo() {}

}  // namespace nu
