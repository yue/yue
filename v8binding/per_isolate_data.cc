// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "v8binding/per_isolate_data.h"

// The slot 3 had been reserved for Node, but since it is not using the slot
// after Node v7, we are free to take it.
#ifndef V8BINDING_ISOLATE_SLOT
#define V8BINDING_ISOLATE_SLOT 3
#endif

namespace vb {

// static
PerIsolateData* PerIsolateData::Get(v8::Isolate* isolate) {
  void* data = isolate->GetData(V8BINDING_ISOLATE_SLOT);
  if (!data) {
    data = new PerIsolateData(isolate);
    isolate->SetData(V8BINDING_ISOLATE_SLOT, data);
  }
  return static_cast<PerIsolateData*>(data);
}

PerIsolateData::PerIsolateData(v8::Isolate* isolate)
    : isolate_(isolate) {
}

PerIsolateData::~PerIsolateData() {
  // This class is simply leaked.
}

void PerIsolateData::SetFunctionTemplate(
    const char* name,
    v8::Local<v8::FunctionTemplate> tmpl) {
  function_templates_[name] = v8::Eternal<v8::FunctionTemplate>(isolate_, tmpl);
}

v8::Local<v8::FunctionTemplate> PerIsolateData::GetFunctionTemplate(
    const char* name) {
  auto it = function_templates_.find(name);
  if (it == function_templates_.end())
    return v8::Local<v8::FunctionTemplate>();
  return it->second.Get(isolate_);
}

}  // namespace vb
