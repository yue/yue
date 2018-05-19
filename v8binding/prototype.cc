// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "v8binding/prototype.h"

#include "v8binding/per_isolate_data.h"

namespace vb {

namespace internal {

namespace {

void DefaultConstructor(const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() < 1 || !info[0]->IsExternal()) {
    const char* message = "The constructor of this class can not be used.";
    v8::Isolate* isolate = info.GetIsolate();
    isolate->ThrowException(v8::Exception::TypeError(
        ToV8(isolate->GetCurrentContext(), message).As<v8::String>()));
  }
}

}  // namespace

ObjectTracker::ObjectTracker(v8::Isolate* isolate, v8::Local<v8::Object> obj)
    : isolate_(isolate), v8_ref_(isolate, obj) {
  v8_ref_.SetWeak(this, &ObjectTracker::WeakCallback,
                  v8::WeakCallbackType::kParameter);
}

ObjectTracker::~ObjectTracker() {
  DCHECK(!v8_ref_.IsEmpty());
}

v8::Local<v8::Object> ObjectTracker::GetHandle() const {
  return v8::Local<v8::Object>::New(isolate_, v8_ref_);
}

v8::Isolate* ObjectTracker::GetIsolate() const {
  return isolate_;
}

// static
void ObjectTracker::WeakCallback(
    const v8::WeakCallbackInfo<ObjectTracker>& data) {
  delete data.GetParameter();
}

bool GetOrCreateFunctionTemplate(
    v8::Isolate* isolate,
    void* key,
    v8::Local<v8::FunctionTemplate>* templ) {
  auto* per_isolate_data = PerIsolateData::Get(isolate);
  *templ = per_isolate_data->GetFunctionTemplate(key);
  if (templ->IsEmpty()) {
    *templ = v8::FunctionTemplate::New(isolate, &DefaultConstructor);
    per_isolate_data->SetFunctionTemplate(key, *templ);
    return false;
  } else {
    return true;
  }
}

}  // namespace internal

}  // namespace vb
