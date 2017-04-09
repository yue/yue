// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "v8binding/arguments.h"

#include "base/strings/stringprintf.h"

namespace vb {

namespace {

std::string V8TypeAsString(v8::Isolate* isolate, v8::Local<v8::Value> value) {
  if (value.IsEmpty())
    return "<empty handle>";
  if (value->IsUndefined())
    return "undefined";
  if (value->IsNull())
    return "null";
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  std::string result;
  if (value->IsObject()) {
    if (FromV8(context, value.As<v8::Object>()->GetConstructorName(), &result))
      return result;
  }
  if (FromV8(context, value, &result))
    return '"' + result + '"';
  if (FromV8(context, value->TypeOf(isolate), &result))
    return result;
  return "<unkown>";
}

}  // namespace

Arguments::Arguments()
    : isolate_(nullptr),
      info_(nullptr),
      next_(0),
      insufficient_arguments_(false) {
}

Arguments::Arguments(const v8::FunctionCallbackInfo<v8::Value>& info)
    : isolate_(info.GetIsolate()),
      info_(&info),
      next_(0),
      insufficient_arguments_(false) {
}

Arguments::~Arguments() {
}

void Arguments::ThrowError(const char* target_type_name) const {
  v8::Local<v8::Context> context = isolate_->GetCurrentContext();
  if (insufficient_arguments_)
    return ThrowTypeError(context, "Insufficient number of arguments.");

  ThrowTypeError(context, base::StringPrintf(
      "Error processing argument at index %d, conversion failure from %s to %s",
      next_ - 1,
      V8TypeAsString(isolate_, (*info_)[next_ - 1]).c_str(),
      target_type_name));
}

}  // namespace vb
