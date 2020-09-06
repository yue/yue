// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nativeui/win/util/scoped_ole_initializer.h"

#include "base/check_op.h"
#include "base/logging.h"

namespace nu {

ScopedOleInitializer::ScopedOleInitializer()
    :
#ifndef NDEBUG
      // Using the windows API directly to avoid dependency on platform_thread.
      thread_id_(GetCurrentThreadId()),
#endif
      hr_(OleInitialize(NULL)) {
#ifndef NDEBUG
  if (hr_ == S_FALSE) {
    LOG(WARNING) << "Multiple OleInitialize() calls for thread " << thread_id_;
  } else {
    DCHECK_NE(OLE_E_WRONGCOMPOBJ, hr_) << "Incompatible DLLs on machine";
    DCHECK_NE(RPC_E_CHANGED_MODE, hr_) << "Invalid COM thread model change";
  }
#endif
}

ScopedOleInitializer::~ScopedOleInitializer() {
#ifndef NDEBUG
  DCHECK_EQ(thread_id_, GetCurrentThreadId());
#endif
  if (SUCCEEDED(hr_))
    OleUninitialize();
}

}  // namespace nu
