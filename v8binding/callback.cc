// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "v8binding/callback.h"

#include "base/notreached.h"
#include "base/time/time.h"

namespace vb {

namespace internal {

#ifndef NDEBUG
void FunctionTemplateCreated() {
  static base::Time first_time;
  if (first_time.is_null())
    first_time = base::Time::Now();
  if ((base::Time::Now() - first_time).InSeconds() > 10)
    NOTREACHED() << "Creating FunctionTemplate after program has started for a "
                    "while, it is very likely we are leaking FunctionTemplate.";
}
#endif

CallbackHolderBase::CallbackHolderBase(v8::Isolate* isolate)
    : v8_ref_(isolate, v8::External::New(isolate, this)) {
  v8_ref_.SetWeak(this, &CallbackHolderBase::FirstWeakCallback,
                  v8::WeakCallbackType::kParameter);
}

CallbackHolderBase::~CallbackHolderBase() {
  DCHECK(v8_ref_.IsEmpty());
}

v8::Local<v8::External> CallbackHolderBase::GetHandle(v8::Isolate* isolate) {
  return v8::Local<v8::External>::New(isolate, v8_ref_);
}

// static
void CallbackHolderBase::FirstWeakCallback(
    const v8::WeakCallbackInfo<CallbackHolderBase>& data) {
  data.GetParameter()->v8_ref_.Reset();
  data.SetSecondPassCallback(SecondWeakCallback);
}

// static
void CallbackHolderBase::SecondWeakCallback(
    const v8::WeakCallbackInfo<CallbackHolderBase>& data) {
  delete data.GetParameter();
}

V8FunctionWrapper::V8FunctionWrapper(v8::Isolate* isolate,
                                     v8::Local<v8::Function> v8_ref)
    : v8_ref_(isolate, v8_ref) {
}

V8FunctionWrapper::~V8FunctionWrapper() {
}

v8::Local<v8::Function> V8FunctionWrapper::Get(v8::Isolate* isolate) const {
  if (v8_ref_.IsEmpty())
    return v8::Local<v8::Function>();
  return v8::Local<v8::Function>::New(isolate, v8_ref_);
}

}  // namespace internal

}  // namespace vb
