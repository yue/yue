// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_SUBWIN_HOLDER_H_
#define NATIVEUI_WIN_SUBWIN_HOLDER_H_

#include "nativeui/win/window_impl.h"

namespace base {
template<typename T> struct DefaultSingletonTraits;
}

namespace nu {

// Windows does not allow a child window to created without a parent, so this
// window becomes the temporary parent for the SubwinView childs that are not
// added to any parent yet.
class SubwinHolder : public WindowImpl {
 public:
  static SubwinHolder* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<SubwinHolder>;

  SubwinHolder();
  ~SubwinHolder() override;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_SUBWIN_HOLDER_H_
