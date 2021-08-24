// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_BINDING_SIGNAL_H_
#define NODE_YUE_BINDING_SIGNAL_H_

#include <utility>

#include "nativeui/nativeui.h"
#include "v8binding/v8binding.h"

namespace node_yue {

template<typename Sig>
class SignalWrapper : public base::RefCounted<SignalWrapper<Sig>> {
 public:
  SignalWrapper(v8::Isolate* isolate,
                v8::Local<v8::Object> owner,
                nu::Signal<Sig>* signal)
      : owner_(isolate, owner), signal_(signal) {
    owner_.SetWeak();
  }

  int Connect(vb::Arguments* args, v8::Local<v8::Value> value) {
    v8::Isolate* isolate = args->isolate();
    if (owner_.IsEmpty()) {
      vb::ThrowError(isolate, "Signal owner is garbage collected");
      return -1;
    }
    // The signal handler must not be referenced by C++.
    v8::Local<v8::Context> context = args->GetContext();
    std::function<Sig> slot;
    if (!vb::WeakFunctionFromV8(context, value, &slot)) {
      args->ThrowError("Function");
      return -1;
    }
    int id = signal_->Connect(std::move(slot));
    // owner[signal][id] = slot.
    v8::Local<v8::Map> refs = vb::GetAttachedTable(
        context, v8::Local<v8::Object>::New(isolate, owner_), signal_);
    refs->Set(context, v8::Integer::New(isolate, id), value).IsEmpty();
    return id;
  }

  void Disconnect(vb::Arguments* args, int id) {
    v8::Isolate* isolate = args->isolate();
    if (owner_.IsEmpty()) {
      vb::ThrowError(isolate, "Signal owner is garbage collected");
      return;
    }
    signal_->Disconnect(id);
    // delete owner[signal][id]
    v8::Local<v8::Context> context = args->GetContext();
    v8::Local<v8::Map> refs = vb::GetAttachedTable(
        context, v8::Local<v8::Object>::New(isolate, owner_), signal_);
    refs->Delete(context, v8::Integer::New(isolate, id)).FromJust();
  }

  void DisconnectAll(vb::Arguments* args) {
    v8::Isolate* isolate = args->isolate();
    if (owner_.IsEmpty()) {
      vb::ThrowError(isolate, "Signal owner is garbage collected");
      return;
    }
    signal_->DisconnectAll();
    // owner[signal].clear()
    v8::Local<v8::Context> context = args->GetContext();
    vb::GetAttachedTable(context,
                         v8::Local<v8::Object>::New(isolate, owner_),
                         signal_)->Clear();
  }

 private:
  ~SignalWrapper() {}

  friend class base::RefCounted<SignalWrapper<Sig>>;

  v8::Global<v8::Object> owner_;
  nu::Signal<Sig>* signal_;
};

}  // namespace node_yue

namespace vb {

// Converter for Signal.
template<typename Sig>
struct Type<node_yue::SignalWrapper<Sig>> {
  static constexpr const char* name = "Signal";
  static void BuildConstructor(v8::Local<v8::Context>, v8::Local<v8::Object>) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
    Set(context, templ,
        "connect", &node_yue::SignalWrapper<Sig>::Connect,
        "disconnect", &node_yue::SignalWrapper<Sig>::Disconnect,
        "disconnectAll", &node_yue::SignalWrapper<Sig>::DisconnectAll);
  }
};

// Define how the Signal member is converted.
template<typename Sig>
struct MemberTraits<nu::Signal<Sig>> {
  static const RefMode kRefMode = RefMode::FirstGet;
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   v8::Local<v8::Object> owner,
                                   const nu::Signal<Sig>& signal) {
    v8::Isolate* isolate = context->GetIsolate();
    return vb::ToV8(context, new node_yue::SignalWrapper<Sig>(
        isolate, owner, const_cast<nu::Signal<Sig>*>(&signal)));
  }
  static inline bool FromV8(v8::Local<v8::Context> context,
                            v8::Local<v8::Object> owner,
                            v8::Local<v8::Value> value,
                            nu::Signal<Sig>* out) {
    if (!value->IsFunction())
      return false;
    // The signal handler must not be referenced by C++.
    std::function<Sig> slot;
    if (!vb::WeakFunctionFromV8(context, value, &slot))
      return false;
    int id = out->Connect(std::move(slot));
    // owner[signal][id] = slot
    v8::Local<v8::Map> refs = vb::GetAttachedTable(context, owner, out);
    refs->Set(context,
              v8::Integer::New(context->GetIsolate(), id),
              value).IsEmpty();
    return true;
  }
};

}  // namespace vb

#endif  // NODE_YUE_BINDING_SIGNAL_H_
