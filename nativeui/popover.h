// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_POPOVER_H_
#define NATIVEUI_POPOVER_H_

#include "base/memory/ref_counted.h"
#include "nativeui/signal.h"
#include "nativeui/types.h"

namespace nu {

class PopoverRootView;
class SizeF;
class View;
class Window;

class NATIVEUI_EXPORT Popover : public base::RefCounted<Popover> {
 public:
  Popover();

  void ShowRelativeTo(View* view);
  void Close();
  void SetContentView(scoped_refptr<View> view);
  View* GetContentView() const;
  void SetContentSize(const SizeF& size);

  // Events.
  Signal<void(Popover*)> on_close;

 protected:
  virtual ~Popover();

 private:
  friend class base::RefCounted<Popover>;

  scoped_refptr<PopoverRootView> root_view_;
  scoped_refptr<Window> window_;
};

}  // namespace nu

#endif  // NATIVEUI_POPOVER_H_
