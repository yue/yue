// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_LAYOUT_LAYOUT_MANAGER_H_
#define NATIVEUI_LAYOUT_LAYOUT_MANAGER_H_

#include "nativeui/view.h"

namespace nu {

NATIVEUI_EXPORT class LayoutManager : public base::RefCounted<LayoutManager> {
 public:
  explicit LayoutManager(View* host);

  virtual void Layout() = 0;

 protected:
  virtual ~LayoutManager();

  View* host() const { return host_; }

 private:
  friend class base::RefCounted<LayoutManager>;

  View* host_;  // weak ptr.
};

}  // namespace nu

#endif  // NATIVEUI_LAYOUT_LAYOUT_MANAGER_H_
