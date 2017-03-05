// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <node.h>

#include "native_mate/dictionary.h"
#include "native_mate/object_template_builder.h"
#include "native_mate/wrappable.h"
#include "nativeui/nativeui.h"

namespace node_yue {

class Lifetime : public mate::Wrappable<Lifetime> {
 public:
  explicit Lifetime(v8::Isolate* isolate) {
    Init(isolate);
  }

  static void BuildPrototype(v8::Isolate* isolate,
                             v8::Local<v8::FunctionTemplate> prototype) {
    prototype->SetClassName(mate::StringToV8(isolate, "Lifetime"));
    mate::ObjectTemplateBuilder(isolate, prototype->PrototypeTemplate())
        .SetMethod("run", &Lifetime::Run)
        .SetMethod("quit", &Lifetime::Quit);
  }

  void Run() {
    lifetime_.Run();
  }

  void Quit() {
    lifetime_.Quit();
    delete this;
  }

 private:
  nu::Lifetime lifetime_;
};

void Initialize(v8::Local<v8::Object> exports) {
  // Initialize the nativeui and leak it.
  new nu::State;
  // Create bindings.
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  mate::Dictionary dict(isolate, exports);
  dict.Set("lifetime", (new Lifetime(isolate))->GetWrapper());
}

}  // namespace node_yue

NODE_MODULE(yue, node_yue::Initialize)
