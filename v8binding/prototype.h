// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef V8BINDING_PROTOTYPE_H_
#define V8BINDING_PROTOTYPE_H_

#include <type_traits>

#include "v8binding/prototype_internal.h"

namespace vb {

// Generate prototype for native classes.
template<typename T, typename Enable = void>
struct Prototype;

// Create prototype for RefCounted classes.
template<typename T>
struct Prototype<T, typename std::enable_if<std::is_base_of<
                        base::subtle::RefCountedBase, T>::value>::type> {
  static v8::Local<v8::Function> Get(v8::Local<v8::Context> context) {
    auto templ = internal::InheritanceChain<T>::Get(context);
    auto result = templ->GetFunction(context);
    if (result.IsEmpty())
      return v8::Local<v8::Function>();
    else
      return result.ToLocalChecked();
  }

  // Check if current prototype is a base class of the passed one.
  static bool IsBaseOf(v8::Local<v8::Context> context,
                       v8::Local<v8::Object> derived) {
    if (Get(context) == derived)
      return true;
    v8::Local<v8::Object> upper = derived->GetPrototype().As<v8::Object>();
    if (upper.IsEmpty() || upper->IsNull())
      return false;
    return IsBaseOf(context, upper);
  }
};

// The default type information for RefCounted class.
template<typename T>
struct Type<T*, typename std::enable_if<std::is_base_of<
                    base::subtle::RefCountedBase, T>::value>::type> {
  static constexpr const char* name = Type<T>::name;
  static bool FromV8(v8::Local<v8::Context> context,
                     v8::Local<v8::Value> value,
                     T** out) {
    // Verify the type.
    if (!value->IsObject())
      return false;
    v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(value);
    if (obj->InternalFieldCount() != 1)
      return false;
    // Verify fine the inheritance chain.
    if (!Prototype<T>::IsBaseOf(context, obj->GetPrototype().As<v8::Object>()))
      return false;
    // Convert pointer to actual class.
    *out = static_cast<T*>(obj->GetAlignedPointerFromInternalField(0));
    return true;
  }
};

}  // namespace vb

#endif  // V8BINDING_PROTOTYPE_H_
