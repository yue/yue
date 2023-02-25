// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/table/nu_boxed_value.h"

#include <utility>

namespace nu {

base::Value* nu_boxed_value_new(base::Value value) {
  return new base::Value(std::move(value));
}

void nu_boxed_value_free(base::Value* self) {
  delete self;
}

base::Value* nu_boxed_value_copy(base::Value *self) {
  return new base::Value(self->Clone());
}

G_DEFINE_BOXED_TYPE(NUBoxedValue, nu_boxed_value,
                    nu_boxed_value_copy, nu_boxed_value_free)

}  // namespace nu
