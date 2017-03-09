// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_SIGNAL_INTERNAL_H_
#define NODE_YUE_SIGNAL_INTERNAL_H_

#include "v8binding/v8binding.h"

namespace vb {

// Describe how a member T can be converted.
template<typename T>
struct MemberTraits {
  // How the member is converted from V8.
  static inline bool FromV8(v8::Local<v8::Context> context,
                            v8::Local<v8::Value> value,
                            T* out) {
    return vb::FromV8(context, value, out);
  }
  // Decides should we return cached value or converted value.
  static const bool kShouldCacheValue = true;
  // Converter used when we decide not to use cached value.
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          const T&) {
    return v8::Undefined(context->GetIsolate());
  }
};

namespace internal {

// Get type from member pointer.
template<typename T> struct ExtractMemberPointer;
template<typename TType, typename TMember>
struct ExtractMemberPointer<TMember(TType::*)> {
  using ClassType = TType;
  using MemberType = TMember;
};

// A wrapper that stores the V8 member and C++ member pointer.
template<typename T>
class MemberHolder : public base::RefCounted<MemberHolder<T>> {
 public:
  explicit MemberHolder(T ptr) : ptr_(ptr) {}

  static void Getter(v8::Local<v8::String> property,
                     const v8::PropertyCallbackInfo<v8::Value>& info);
  static void Setter(v8::Local<v8::String> property,
                     v8::Local<v8::Value> value,
                     const v8::PropertyCallbackInfo<void>& info);

 private:
  friend class base::RefCounted<MemberHolder<T>>;

  using ClassType = typename ExtractMemberPointer<T>::ClassType;
  using MemberType = typename ExtractMemberPointer<T>::MemberType;

  ~MemberHolder() {}

  T ptr_;
  v8::Global<v8::Value> value;
};

// static
template<typename T>
void MemberHolder<T>::Getter(v8::Local<v8::String> property,
                             const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
  MemberHolder<T>* holder;
  if (!FromV8(context, info.Data(), &holder)) {
    vb::ThrowTypeError(context, "Unable to get holder in Getter");
    return;
  }
  ClassType* instance;
  if (!FromV8(context, info.This(), &instance)) {
    // We should throw here, but Electron is accessing Getter in a strange way
    // that this fails very often.
    return;
  }

  if (MemberTraits<MemberType>::kShouldCacheValue) {
    if (!holder->value.IsEmpty()) {
      info.GetReturnValue().Set(
          v8::Local<v8::Value>::New(info.GetIsolate(), holder->value));
    }
  } else {
    info.GetReturnValue().Set(
        MemberTraits<MemberType>::ToV8(context, instance->*(holder->ptr_)));
  }
}

// static
template<typename T>
void MemberHolder<T>::Setter(v8::Local<v8::String> property,
                             v8::Local<v8::Value> value,
                             const v8::PropertyCallbackInfo<void>& info) {
  v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
  MemberHolder<T>* holder;
  if (!FromV8(context, info.Data(), &holder)) {
    vb::ThrowTypeError(context, "Unable to get holder in Setter");
    return;
  }
  ClassType* instance;
  if (!FromV8(context, info.This(), &instance)) {
    vb::ThrowTypeError(context, "Unable to get instance in Setter");
    return;
  }

  if (!MemberTraits<MemberType>::FromV8(
          context, value, &(instance->*holder->ptr_))) {
    vb::ThrowTypeError(context, "Invalid value passed to Setter");
    return;
  }
  if (MemberTraits<MemberType>::kShouldCacheValue) {
    holder->value = v8::Global<v8::Value>(info.GetIsolate(), value);
  }
}

}  // namespace internal

template<typename T>
struct Type<internal::MemberHolder<T>> {
  static constexpr const char* name = "MemberHolder";
  static void BuildConstructor(v8::Local<v8::Context>, v8::Local<v8::Object>) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
  }
};

}  // namespace vb

#endif  // NODE_YUE_SIGNAL_INTERNAL_H_
