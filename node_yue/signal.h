// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_SIGNAL_H_
#define NODE_YUE_SIGNAL_H_

#include "node_yue/signal_internal.h"

namespace vb {

// Set getter/setter for a member data of class.
template<typename T>
void SetProperty(v8::Local<v8::Context> context,
                 v8::Local<v8::ObjectTemplate> templ,
                 base::StringPiece key,
                 T ptr) {
  v8::Local<v8::Value> holder =
      vb::Prototype<internal::MemberHolder<T>>::template NewInstance<T>(
          context, ptr);
  templ->SetAccessor(vb::ToV8(context, key).As<v8::String>(),
                     &internal::MemberHolder<T>::Getter,
                     &internal::MemberHolder<T>::Setter,
                     holder);
}

// Allow setting arbitrary members.
template<typename T, typename... ArgTypes>
inline void SetProperty(v8::Local<v8::Context> context,
                        v8::Local<v8::ObjectTemplate> templ,
                        base::StringPiece key, T ptr,
                        const ArgTypes&... args) {
  SetProperty(context, templ, key, ptr);
  SetProperty(context, templ, args...);
}

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
    base::Callback<Sig> callback;
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

// Define how the Signal member is converted.
template<typename Sig>
struct MemberTraits<nu::Signal<Sig>> {
  static const bool kShouldCacheValue = false;
  static v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                   v8::Global<v8::Value>* holder_value,
                                   const nu::Signal<Sig>& signal) {
    if (!holder_value->IsEmpty())
      return v8::Local<v8::Value>::New(context->GetIsolate(), *holder_value);
    auto result = internal::CallConstructor<nu::Signal<Sig>>(context);
    if (result.IsEmpty())
      return v8::Null(context->GetIsolate());
    // Store the pointer of signal in the object.
    v8::Local<v8::Object> obj = result.ToLocalChecked();
    obj->SetAlignedPointerInInternalField(
        0, const_cast<nu::Signal<Sig>*>(&signal));
    holder_value->Reset(context->GetIsolate(), obj);
    return obj;
  }
};

}  // namespace vb

#endif  // NODE_YUE_SIGNAL_H_
