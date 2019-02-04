// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_DRAGGING_INFO_WIN_H_
#define NATIVEUI_WIN_DRAGGING_INFO_WIN_H_

#include "nativeui/dragging_info.h"

struct IDataObject;

namespace nu {

class NATIVEUI_EXPORT DraggingInfoWin : public DraggingInfo {
 public:
  DraggingInfoWin(IDataObject* data, int effect);
  ~DraggingInfoWin() override;

  // DraggingInfo:
  bool IsDataAvailable(Data::Type type) const override;
  Data GetData(Data::Type type) const override;

 private:
  IDataObject* data_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_DRAGGING_INFO_WIN_H_
