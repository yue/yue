// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/dragging_info.h"

namespace nu {

DragOptions::DragOptions() {}

DragOptions::DragOptions(Image* image) : image(image) {}

DragOptions::~DragOptions() {}

DraggingInfo::DraggingInfo(int drag_operations)
    : drag_operations_(drag_operations), weak_factory_(this) {}

DraggingInfo::~DraggingInfo() {}

}  // namespace nu
