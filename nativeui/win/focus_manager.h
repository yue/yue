// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_FOCUS_MANAGER_H_
#define NATIVEUI_WIN_FOCUS_MANAGER_H_

#include "nativeui/nativeui_export.h"

namespace nu {

class ViewImpl;
class ContainerImpl;

// Moves focus between views.
class FocusManager {
 public:
  FocusManager();
  ~FocusManager();

  FocusManager& operator=(const FocusManager&) = delete;
  FocusManager(const FocusManager&) = delete;

  // Reset current focus tracking if the view is focused.
  void RemoveFocus(ViewImpl* view);

  // A view has taken the focus by clicking.
  void TakeFocus(ViewImpl* view);

  // Advances the focus inside a view (backward if reverse is true).
  // Returns whether a view has been focused.
  void AdvanceFocus(ViewImpl* view, bool reverse);

  void set_show_focus_ring(bool b) { show_focus_ring_ = b; }
  bool show_focus_ring() const { return show_focus_ring_; }

  ViewImpl* focused_view() const { return focused_view_; }

 private:
  bool DoAdvanceFocus(ContainerImpl* container, bool reverse,
                      bool* focus_on_next_view);

  bool show_focus_ring_ = false;
  ViewImpl* focused_view_ = nullptr;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_FOCUS_MANAGER_H_
