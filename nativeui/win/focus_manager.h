// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_FOCUS_MANAGER_H_
#define NATIVEUI_WIN_FOCUS_MANAGER_H_

#include "base/macros.h"
#include "nativeui/nativeui_export.h"

namespace nu {

class ViewImpl;
class ContainerImpl;

// Moves focus between views.
class FocusManager {
 public:
  FocusManager();
  ~FocusManager();

  // A view has taken the focus by clicking.
  void TakeFocus(ViewImpl* view);

  // Advances the focus inside a view (backward if reverse is true).
  // Returns whether a view has been focused.
  void AdvanceFocus(ViewImpl* view, bool reverse);

  ViewImpl* focused_view() const { return focused_view_; }

 private:
  bool DoAdvanceFocus(ContainerImpl* container, bool reverse,
                      bool* focus_on_next_view);

  ViewImpl* focused_view_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(FocusManager);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_FOCUS_MANAGER_H_
