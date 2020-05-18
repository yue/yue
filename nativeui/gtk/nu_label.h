// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_NU_LABEL_H_
#define NATIVEUI_GTK_NU_LABEL_H_

#include <gtk/gtk.h>

// Custom widget to show text.

namespace nu {

class Label;

#define NU_TYPE_LABEL (nu_label_get_type ())
#define NU_LABEL(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
                       NU_TYPE_LABEL, NULabel))

typedef struct _NULabel        NULabel;
typedef struct _NULabelPrivate NULabelPrivate;
typedef struct _NULabelClass   NULabelClass;

struct _NULabel {
  GtkWidget widget;
  NULabelPrivate* priv;
};

struct _NULabelClass {
  GtkWidgetClass parent_class;
};

GType nu_label_get_type();
GtkWidget* nu_label_new(Label* label);

}  // namespace nu

#endif  // NATIVEUI_GTK_NU_LABEL_H_
