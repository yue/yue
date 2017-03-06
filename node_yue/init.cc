// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <node.h>

#include "nativeui/nativeui.h"
#include "v8binding/v8binding.h"

namespace vb {

template<>
struct Type<nu::Window::Options> {
  static constexpr const char* name = "yue.Window.Options";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Window::Options* out) {
    return true;
  }
};

template<>
struct Type<nu::Window> {
  static constexpr const char* name = "yue.Window";
  static void BuildConstructor(v8::Local<v8::Context> context,
                               v8::Local<v8::Object> constructor) {
    Set(context, constructor,
        "create", &Prototype<nu::Window>::NewInstance<nu::Window::Options>);
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "close", &nu::Window::Close);
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
  vb::Set(context, exports,
          "Window", vb::Prototype<nu::Window>::Get(context));
}

}  // namespace node_yue

NODE_MODULE(yue, node_yue::Initialize)
