// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_CURSOR_H_
#define NATIVEUI_CURSOR_H_

#include "base/memory/ref_counted.h"
#include "nativeui/nativeui_export.h"
#include "nativeui/types.h"

namespace nu {

// Wrapper of native cursor.
class NATIVEUI_EXPORT Cursor : public base::RefCounted<Cursor> {
 public:
  enum class Type {
    Default,
    Hand,
    Crosshair,
    Progress,
    Text,
    NotAllowed,
    Help,
    Move,
    ResizeEW,
    ResizeNS,
    ResizeNESW,
    ResizeNWSE,
  };

  explicit Cursor(Type type = Type::Default);

  NativeCursor GetNative() const { return cursor_; }

 private:
  friend class base::RefCounted<Cursor>;

  ~Cursor();

  NativeCursor cursor_;
};

}  // namespace nu

#endif  // NATIVEUI_CURSOR_H_
