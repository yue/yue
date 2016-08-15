// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.
//
// Scoped pointers for native types.

#ifndef NATIVEUI_SCOPED_TYPES_H_
#define NATIVEUI_SCOPED_TYPES_H_

#include <memory>

#include "base/macros.h"
#include "nativeui/types.h"

namespace nu {

#if defined(OS_MACOSX)
template<typename T>
struct ScopedNSObjectImpl;

template<typename T>
class ScopedNSObject {
 public:
  ScopedNSObject();
  ~ScopedNSObject();

  void Reset(T* object);
  T* Get() const;

 private:
  std::unique_ptr<ScopedNSObjectImpl<T>> impl_;

  DISALLOW_COPY_AND_ASSIGN(ScopedNSObject);
};

using ScopedNativeWindow =
    ScopedNSObject<std::remove_pointer<NativeWindow>::type>;
using ScopedNativeView =
    ScopedNSObject<std::remove_pointer<NativeView>::type>;
#endif

}  // namespace nu

#endif  // NATIVEUI_SCOPED_TYPES_H_
