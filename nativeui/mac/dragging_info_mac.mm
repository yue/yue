// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/dragging_info_mac.h"

namespace nu {

DraggingInfoMac::DraggingInfoMac(id<NSDraggingInfo> info)
   : DraggingInfo([info draggingSourceOperationMask]),
     // According to Apple:
     // Although a standard dragging pasteboard (obtained using NSDragPboard) is
     // provided as a convenience in getting the pasteboard for dragging data,
     // there is NO guarantee that this will be the pasteboard used in a cross-
     // process drag. Thus, to guarantee getting the correct pasteboard, your
     // code should use [sender draggingPasteboard].
     clipboard_([info draggingPasteboard]) {}

DraggingInfoMac::~DraggingInfoMac() {}

bool DraggingInfoMac::IsDataAvailable(Data::Type type) const {
  return clipboard_.IsDataAvailable(type);
}

Clipboard::Data DraggingInfoMac::GetData(Data::Type type) const {
  return clipboard_.GetData(type);
}

}  // namespace nu
