// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_SCOPED_TYPES_MAC_H_
#define NATIVEUI_SCOPED_TYPES_MAC_H_

#include "nativeui/scoped_types.h"

#include "base/mac/scoped_nsobject.h"

namespace nu {

template<typename T>
struct ScopedNSObjectImpl {
  base::scoped_nsobject<T> object;
};

template<typename T>
ScopedNSObject<T>::ScopedNSObject() : impl_(new ScopedNSObjectImpl<T>()) {
}

template<typename T>
ScopedNSObject<T>::~ScopedNSObject() {
}

template<typename T>
void ScopedNSObject<T>::Reset(T* object) {
  impl_->object.reset(object);
}

template<typename T>
T* ScopedNSObject<T>::Get() const {
  return impl_->object.get();
}

}  // namespace nu

#endif  // NATIVEUI_SCOPED_TYPES_MAC_H_
