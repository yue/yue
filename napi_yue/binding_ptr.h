// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NAPI_YUE_BINDING_PTR_H_
#define NAPI_YUE_BINDING_PTR_H_

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "third_party/kizunapi/kizunapi.h"

namespace ki {

template<typename T>
struct Type<scoped_refptr<T>> {
  static constexpr const char* name = Type<T>::name;
  static napi_status ToNode(napi_env env,
                            const scoped_refptr<T>& ptr,
                            napi_value* result) {
    return ConvertToNode(env, ptr.get(), result);
  }
  static napi_status FromNode(napi_env env,
                              napi_value value,
                              scoped_refptr<T>* out) {
    T* out_ptr;
    napi_status s = ConvertFromNode(env, value, &out_ptr);
    if (s == napi_ok)
      *out = out_ptr;
    return s;
  }
};

// Do automatic ref for RefCounted types.
template<typename T>
struct TypeBridge<T, typename std::enable_if<std::is_base_of<
                         base::subtle::RefCountedBase, T>::value>::type> {
  static T* Wrap(T* ptr) {
    ptr->AddRef();
    return ptr;
  }
  static void Finalize(T* ptr) {
    ptr->Release();
  }
};

// For classes that produce weak ptrs, store weak ptr in JS object.
template<typename T>
struct TypeBridge<T, typename std::enable_if<std::is_base_of<
                         base::internal::WeakPtrBase,
                         decltype(((T*)nullptr)->GetWeakPtr())>::value>::type> {  // NOLINT
  static constexpr bool can_cache_pointer = false;
  static base::WeakPtr<T>* Wrap(T* ptr) {
    return new base::WeakPtr<T>(ptr->GetWeakPtr());
  }
  static T* Unwrap(base::WeakPtr<T>* data) {
    return data->get();
  }
  static void Finalize(base::WeakPtr<T>* data) {
    delete data;
  }
};

}  // namespace ki

#endif  // NAPI_YUE_BINDING_PTR_H_
