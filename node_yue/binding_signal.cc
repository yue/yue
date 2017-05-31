// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "node_yue/binding_signal.h"

namespace node_yue {

OwnerTracker::OwnerTracker(v8::Isolate* isolate,
                           v8::Local<v8::Object> owner,
                           v8::Local<v8::Object> holder)
    : vb::internal::ObjectTracker(isolate, owner),
      holder_ref_(isolate, holder) {
  holder_ref_.SetWeak();
}

OwnerTracker::~OwnerTracker() {
  if (!holder_ref_.IsEmpty()) {
    v8::Local<v8::Object>::New(GetIsolate(), holder_ref_)->
        SetAlignedPointerInInternalField(0, nullptr);
  }
}

}  // namespace node_yue
