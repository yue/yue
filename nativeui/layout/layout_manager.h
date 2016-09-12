// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LAYOUT_LAYOUT_MANAGER_H_
#define NATIVEUI_LAYOUT_LAYOUT_MANAGER_H_

#include "nativeui/container.h"

namespace nu {

NATIVEUI_EXPORT class LayoutManager : public base::RefCounted<LayoutManager> {
 public:
  // Lay out the children of |host| according to implementation-specific
  // heuristics. The graphics used during painting is provided to allow for
  // string sizing.
  virtual void Layout(Container* host) const = 0;

  // Return the preferred size which is the size required to give each
  // children their respective preferred size.
  virtual Size GetPixelPreferredSize(Container* host) const = 0;

  // Notification that a view has been added.
  virtual void ViewAdded(View* host, View* view) {}

  // Notification that a view has been removed.
  virtual void ViewRemoved(View* host, View* view) {}

 protected:
  LayoutManager() {}
  virtual ~LayoutManager() {}

 private:
  friend class base::RefCounted<LayoutManager>;
};

}  // namespace nu

#endif  // NATIVEUI_LAYOUT_LAYOUT_MANAGER_H_
