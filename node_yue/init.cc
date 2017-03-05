// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <node.h>

#include "nativeui/nativeui.h"
#include "v8binding/v8binding.h"

namespace vb {

template<>
struct Type<nu::Window> {
  static constexpr const char* name = "yue.Window";
  static void BuildPrototype(v8::Isolate* isolate) {
  }
};

}  // namespace vb

namespace node_yue {

void Initialize(v8::Local<v8::Object> exports) {
  // Initialize the nativeui and leak it.
  new nu::State;

  // Populate with GUI elements.
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  vb::Dict(context, exports)
      .Set("Window", vb::Prototype<nu::Window>::Get(context));
}

}  // namespace node_yue

NODE_MODULE(yue, node_yue::Initialize)
