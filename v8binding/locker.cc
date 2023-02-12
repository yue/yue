// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "v8binding/locker.h"

#include "v8.h"  // NOLINT(build/include_directory)

namespace vb {

Locker::Locker(v8::Isolate* isolate) {
#if V8_MAJOR_VERSION >= 10
  if (v8::Locker::IsLocked(isolate))
#else
  if (v8::Locker::IsActive())
#endif
    locker_.reset(new v8::Locker(isolate));
}

Locker::~Locker() {
}

}  // namespace vb
