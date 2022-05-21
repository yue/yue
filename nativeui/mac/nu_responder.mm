// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/nu_responder.h"

#include <objc/objc-runtime.h>

namespace nu {

bool IsNUResponder(id responder) {
  return [responder respondsToSelector:@selector(nuPrivate)];
}

}  // namespace nu
