// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "v8binding/ref_method.h"

namespace vb {

namespace internal {

void StoreArg(v8::Local<v8::Context> context,
              v8::Local<v8::Object> object,
              v8::Local<v8::Value> ref_arg,
              RefType ref_type,
              const char* ref_key) {
  v8::Isolate* isolate = context->GetIsolate();
  auto key = v8::Private::ForApi(isolate, ToV8Symbol(context, "refs"));
  if (!object->HasPrivate(context, key).ToChecked())
    object->SetPrivate(context, key, v8::Map::New(isolate));
  v8::Local<v8::Map> refs = v8::Local<v8::Map>::Cast(
      object->GetPrivate(context, key).ToLocalChecked());
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
