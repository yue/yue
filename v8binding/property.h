// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_PROPERTY_H_
#define V8BINDING_PROPERTY_H_

#include <utility>

#include "v8binding/prototype.h"

namespace vb {

enum class RefMode {
  Always,
  Never,
  FirstGet,
};

// Describe how a member T can be converted.
template<typename T, typename Enable = void>
struct MemberTraits {
  // Decides should we return store a reference to the value.
  static const RefMode kRefMode = RefMode::Always;
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          v8::Local<v8::Object> owner,
                                          const T&) {
    return v8::Undefined(context->GetIsolate());
  }
  static inline bool FromV8(v8::Local<v8::Context> context,
                            v8::Local<v8::Object> owner,
                            v8::Local<v8::Value> value,
                            T* out) {
    return Type<T>::FromV8(context, value, out);
  }
};

// For primary types we just do normal conversion.
template<typename T>
struct MemberTraits<T, typename std::enable_if<
                           std::is_fundamental<T>::value>::type> {
  static const RefMode kRefMode = RefMode::Never;
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          v8::Local<v8::Object> owner,
                                          const T& value) {
    return Type<T>::ToV8(context, value);
  }
  static inline bool FromV8(v8::Local<v8::Context> context,
                            v8::Local<v8::Object> owner,
                            v8::Local<v8::Value> value,
                            T* out) {
    return Type<T>::FromV8(context, value, out);
  }
};

// Convert to weak callbacks for properties.
template<typename Sig>
struct MemberTraits<std::function<Sig>> {
  static const RefMode kRefMode = RefMode::Always;
  static inline v8::Local<v8::Value> ToV8(v8::Local<v8::Context> context,
                                          v8::Local<v8::Object> owner,
                                          const std::function<Sig>&) {
    return v8::Undefined(context->GetIsolate());
  }
  static inline bool FromV8(v8::Local<v8::Context> context,
                            v8::Local<v8::Object> owner,
                            v8::Local<v8::Value> value,
                            std::function<Sig>* out) {
    return WeakFunctionFromV8(context, value, out);
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

  v8::Local<v8::Map> refs;
  if (MemberTraits<MemberType>::kRefMode != RefMode::Never) {
     refs = GetAttachedTable(context, info.This(), "members");
    if (refs->Has(context, property).FromJust() ||
        MemberTraits<MemberType>::kRefMode == RefMode::Always) {
      info.GetReturnValue().Set(refs->Get(context, property).ToLocalChecked());
      return;
    }
  }

  v8::Local<v8::Value> ret = MemberTraits<MemberType>::ToV8(
      context, info.This(), instance->*(holder->ptr_));
  if (MemberTraits<MemberType>::kRefMode == RefMode::FirstGet)
    refs->Set(context, property, ret).IsEmpty();
  info.GetReturnValue().Set(ret);
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
          context, info.This(), value, &(instance->*holder->ptr_))) {
    vb::ThrowTypeError(context, "Invalid value passed to Setter");
    return;
  }

  if (MemberTraits<MemberType>::kRefMode == RefMode::Always) {
    v8::Local<v8::Map> refs = GetAttachedTable(context, info.This(), "members");
    refs->Set(context, property, value).IsEmpty();
  }
}

}  // namespace internal

// Required for binding MemberHolder to V8.
template<typename T>
struct Type<internal::MemberHolder<T>> {
  static constexpr const char* name = "MemberHolder";
  static void BuildConstructor(v8::Local<v8::Context>, v8::Local<v8::Object>) {
  }
  static void BuildPrototype(v8::Local<v8::Context> context,
                             v8::Local<v8::ObjectTemplate> templ) {
  }
};

// Set getter/setter for a member data of class.
template<typename T>
void SetProperty(v8::Local<v8::Context> context,
                 v8::Local<v8::ObjectTemplate> templ,
                 base::StringPiece key,
                 T ptr) {
  templ->SetAccessor(
      ToV8(context, key).As<v8::String>(),
      &internal::MemberHolder<T>::Getter,
      &internal::MemberHolder<T>::Setter,
      ToV8(context, new internal::MemberHolder<T>(ptr)));
}

// Allow setting arbitrary members.
template<typename T, typename... ArgTypes>
inline void SetProperty(v8::Local<v8::Context> context,
                        v8::Local<v8::ObjectTemplate> templ,
                        base::StringPiece key, T ptr,
                        ArgTypes&&... args) {
  SetProperty(context, templ, key, ptr);
  SetProperty(context, templ, std::forward<ArgTypes>(args)...);
}

}  // namespace vb

#endif  // V8BINDING_PROPERTY_H_
