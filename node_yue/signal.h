// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_SIGNAL_H_
#define NODE_YUE_SIGNAL_H_

#include "node_yue/signal_internal.h"

namespace vb {

// Set getter/setter for a member data of class.
template<typename T>
void DefineMember(v8::Local<v8::Context> context,
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

}  // namespace vb

#endif  // NODE_YUE_SIGNAL_H_
