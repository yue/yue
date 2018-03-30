// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "v8binding/dict.h"

#include "base/strings/string_number_conversions.h"

namespace vb {

v8::Local<v8::Map> GetAttachedTable(v8::Local<v8::Context> context,
                                    v8::Local<v8::Object> object,
                                    const base::StringPiece& key_str) {
  v8::Isolate* isolate = context->GetIsolate();
  auto key = v8::Private::ForApi(isolate,
                                 ToV8(context, key_str).As<v8::String>());
  if (!object->HasPrivate(context, key).ToChecked())
    object->SetPrivate(context, key, v8::Map::New(isolate));
  return v8::Local<v8::Map>::Cast(
      object->GetPrivate(context, key).ToLocalChecked());
}

v8::Local<v8::Map> GetAttachedTable(v8::Local<v8::Context> context,
                                    v8::Local<v8::Object> object,
                                    void* key) {
  return GetAttachedTable(context, object,
                          base::HexEncode(&key, sizeof(void*)));
}

}  // namespace vb
