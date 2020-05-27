// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/dragging_info_gtk.h"

#include <utility>

#include "nativeui/gtk/util/clipboard_util.h"

namespace nu {

DraggingInfoGtk::DraggingInfoGtk(GdkDragContext* context)
    : DraggingInfo(gdk_drag_context_get_actions(context)) {}

DraggingInfoGtk::DraggingInfoGtk(GdkDragContext* context,
                                 std::map<Data::Type, Data> data)
    : DraggingInfo(gdk_drag_context_get_actions(context)),
      data_(std::move(data)) {}

DraggingInfoGtk::~DraggingInfoGtk() {}

bool DraggingInfoGtk::IsDataAvailable(Data::Type type) const {
  auto it = data_.find(type);
  return it != data_.end() && it->second.type() != Data::Type::None;
}

Clipboard::Data DraggingInfoGtk::GetData(Data::Type type) const {
  auto it = data_.find(type);
  return it == data_.end() ? Data() : it->second.Clone();
}

}  // namespace nu
