// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "v8binding/locker.h"

#include "v8.h"  // NOLINT(build/include_directory)

namespace vb {

Locker::Locker(v8::Isolate* isolate) {
  if (v8::Locker::IsActive())
    locker_.reset(new v8::Locker(isolate));
}

Locker::~Locker() {
}

}  // namespace vb
