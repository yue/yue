// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_NU_CUSTOM_CELL_RENDERER_H_
#define NATIVEUI_GTK_NU_CUSTOM_CELL_RENDERER_H_

#include <gtk/gtk.h>

#include "nativeui/table.h"

// Custom cell renderer for Table.

namespace nu {

#define NU_TYPE_CUSTOM_CELL_RENDERER (nu_custom_cell_renderer_get_type())
#define NU_CUSTOM_CELL_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
                                      NU_TYPE_CUSTOM_CELL_RENDERER, \
                                      NUCustomCellRenderer))
#define NU_IS_CUSTOM_CELL_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
                                         NU_TYPE_CUSTOM_CELL_RENDERER))

typedef struct _NUCustomCellRenderer        NUCustomCellRenderer;
typedef struct _NUCustomCellRendererPrivate NUCustomCellRendererPrivate;
typedef struct _NUCustomCellRendererClass   NUCustomCellRendererClass;

struct _NUCustomCellRenderer {
  GtkCellRenderer parent;
  NUCustomCellRendererPrivate* priv;
};

struct _NUCustomCellRendererClass {
  GtkCellRendererClass parent_class;
};

GType nu_custom_cell_renderer_get_type();
GtkCellRenderer* nu_custom_cell_renderer_new(
    const Table::ColumnOptions& options);

}  // namespace nu

#endif  // NATIVEUI_GTK_NU_CUSTOM_CELL_RENDERER_H_
