// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_DRAGGING_INFO_MAC_H_
#define NATIVEUI_MAC_DRAGGING_INFO_MAC_H_

#import <Cocoa/Cocoa.h>

#include "nativeui/clipboard.h"
#include "nativeui/dragging_info.h"

namespace nu {

class Clipboard;

class NATIVEUI_EXPORT DraggingInfoMac : public DraggingInfo {
 public:
  explicit DraggingInfoMac(id<NSDraggingInfo> info);
  ~DraggingInfoMac() override;

  // DraggingInfo:
  bool IsDataAvailable(Data::Type type) const override;
  Data GetData(Data::Type type) const override;

 private:
  Clipboard clipboard_;
};

}  // namespace nu

#endif  // NATIVEUI_MAC_DRAGGING_INFO_MAC_H_
