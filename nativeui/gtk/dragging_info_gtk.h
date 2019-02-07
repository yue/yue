// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_DRAGGING_INFO_GTK_H_
#define NATIVEUI_GTK_DRAGGING_INFO_GTK_H_

#include <gtk/gtk.h>

#include <map>

#include "nativeui/dragging_info.h"

namespace nu {

class DraggingInfoGtk : public DraggingInfo {
 public:
  explicit DraggingInfoGtk(GdkDragContext* context);
  DraggingInfoGtk(GdkDragContext* context, std::map<Data::Type, Data> data);
  ~DraggingInfoGtk() override;

  // DraggingInfo:
  bool IsDataAvailable(Data::Type type) const override;
  Data GetData(Data::Type type) const override;

 private:
  std::map<Data::Type, Data> data_;
};

}  // namespace nu

#endif  // NATIVEUI_GTK_DRAGGING_INFO_GTK_H_
