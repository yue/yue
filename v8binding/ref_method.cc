// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "v8binding/ref_method.h"

#include "v8binding/dict.h"

namespace vb {

namespace internal {

void StoreArg(v8::Local<v8::Context> context,
              v8::Local<v8::Object> object,
              v8::Local<v8::Value> ref_arg,
              RefType ref_type,
              const char* ref_key) {
  v8::Isolate* isolate = context->GetIsolate();
  v8::Local<v8::Map> refs = GetAttachedTable(context, object, "refs");
  switch (ref_type) {
    case RefType::Reset:
      refs->Set(context, ToV8(context, ref_key), ref_arg).IsEmpty();
      break;
    case RefType::Ref:
      refs->Set(context, ref_arg, v8::True(isolate)).IsEmpty();
      break;
    case RefType::Deref:
      refs->Delete(context, ref_arg).IsNothing();
      break;
  }
}

}  // namespace internal

}  // namespace vb
