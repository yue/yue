// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_FOCUS_MANAGER_H_
#define NATIVEUI_WIN_FOCUS_MANAGER_H_

#include "nativeui/container.h"

namespace nu {

// Moves focus between views.
class FocusManager {
 public:
  FocusManager();
  ~FocusManager();

  // A view has taken the focus by clicking.
  void TakeFocus(View* view);

  // Advances the focus in container (backward if reverse is true).
  // Returns whether a view has been focused.
  void AdvanceFocus(Container* container, bool reverse);

  View* focused_view() const { return focused_view_; }

 private:
  bool DoAdvanceFocus(Container* container, bool reverse,
                      bool* focus_on_next_view);

  View* focused_view_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(FocusManager);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_FOCUS_MANAGER_H_
