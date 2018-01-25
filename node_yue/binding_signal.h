// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_BINDING_SIGNAL_H_
#define NODE_YUE_BINDING_SIGNAL_H_

#include "nativeui/nativeui.h"
#include "v8binding/v8binding.h"

namespace node_yue {

// Tracks owner's lifetime and releases the holder on GC.
class OwnerTracker : public vb::internal::ObjectTracker {
 public:
  OwnerTracker(v8::Isolate* isolate,
               v8::Local<v8::Object> owner,
               v8::Local<v8::Object> holder);
  ~OwnerTracker() override;

 private:
  v8::Global<v8::Object> holder_ref_;
};

}  // namespace node_yue

namespace vb {

// Converter for Signal.
template<typename Sig>
struct Type<nu::Signal<Sig>> {
  static constexpr const char* name = "yue.Signal";
  static void BuildConstructor(v8::Local<v8::Context>, v8::Local<v8::Object>) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "connect", &nu::Signal<Sig>::Connect,
        "disconnect", &nu::Signal<Sig>::Disconnect,
        "disconnectAll", &nu::Signal<Sig>::DisconnectAll);
  }
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Signal<Sig>* out) {
    if (!value->IsFunction())
      return false;
    std::function<Sig> callback;
    if (!vb::FromV8(context, value, &callback))
      return false;
    out->Connect(callback);
    return true;
  }
};

template<typename Sig>
struct Type<nu::Signal<Sig>*> {
  static constexpr const char* name = "yue.Signal";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::Signal<Sig>** out) {
    // Verify the type.
    if (!value->IsObject())
      return false;
    v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(value);
    if (obj->InternalFieldCount() != 1)
      return false;
    // Convert pointer to actual class.
    auto* ptr = static_cast<nu::Signal<Sig>*>(
        obj->GetAlignedPointerFromInternalField(0));
    if (!ptr)
      return false;
    *out = ptr;
    return true;
  }
};

template<typename Sig>
struct Type<nu::SignalBase<Sig>*> {
  static constexpr const char* name = "yue.Signal";
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     nu::SignalBase<Sig>** out) {
    return Type<nu::Signal<Sig>*>::FromV8(
        context, value, reinterpret_cast<nu::Signal<Sig>**>(out));
  }
};

// Define how the Signal member is converted.
template<typename Sig>
struct MemberTraits<nu::Signal<Sig>> {
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   v8::Local<v8::Object> owner,
                                   const nu::Signal<Sig>& signal) {
    v8::Isolate* isolate = context->GetIsolate();
    auto result = CallConstructor<nu::Signal<Sig>>(context);
    if (result.IsEmpty())
      return v8::Null(isolate);
    // Store the pointer of signal in the object.
    v8::Local<v8::Object> obj = result.ToLocalChecked();
    obj->SetAlignedPointerInInternalField(
        0, const_cast<nu::Signal<Sig>*>(&signal));
    // Track the lifetime of owner.
    new node_yue::OwnerTracker(isolate, owner, obj);
    return obj;
  }
};

}  // namespace vb

#endif  // NODE_YUE_BINDING_SIGNAL_H_
