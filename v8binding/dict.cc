// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "v8binding/dict.h"

namespace vb {

Dict::Dict(v8::Local<v8::Context> context, v8::Local<v8::Object> object)
    : context_(context), object_(object) {
}

Dict::~Dict() {
}

}  // namespace vb
