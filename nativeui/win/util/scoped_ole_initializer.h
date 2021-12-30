// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVEUI_WIN_UTIL_SCOPED_OLE_INITIALIZER_H_
#define NATIVEUI_WIN_UTIL_SCOPED_OLE_INITIALIZER_H_

#include <ole2.h>

namespace nu {

class ScopedOleInitializer {
 public:
  ScopedOleInitializer();
  ~ScopedOleInitializer();

  ScopedOleInitializer& operator=(const ScopedOleInitializer&) = delete;
  ScopedOleInitializer(const ScopedOleInitializer&) = delete;

 private:
#ifndef NDEBUG
  // In debug builds we use this variable to catch a potential bug where a
  // ScopedOleInitializer instance is deleted on a different thread than it
  // was initially created on.  If that ever happens it can have bad
  // consequences and the cause can be tricky to track down.
  DWORD thread_id_;
#endif
  HRESULT hr_;
};

}  // namespace nu

#endif  // NATIVEUI_WIN_UTIL_SCOPED_OLE_INITIALIZER_H_
