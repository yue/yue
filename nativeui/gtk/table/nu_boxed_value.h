// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_TABLE_NU_BOXED_VALUE_H_
#define NATIVEUI_GTK_TABLE_NU_BOXED_VALUE_H_

#include <gtk/gtk.h>

#include "base/values.h"

namespace nu {

#define NU_BOXED_VALUE nu_boxed_value_get_type()
GType nu_boxed_value_get_type() G_GNUC_CONST;

// Standard methods of boxed type.
base::Value* nu_boxed_value_new(base::Value value);
void nu_boxed_value_free(base::Value* self);
base::Value* nu_boxed_value_copy(base::Value* self);

}  // namespace nu

#endif  // NATIVEUI_GTK_TABLE_NU_BOXED_VALUE_H_
