// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_NU_TREE_MODEL_H_
#define NATIVEUI_GTK_NU_TREE_MODEL_H_

#include <gtk/gtk.h>

// Custom tree model type for TableModel.

namespace nu {

class Table;
class TableModel;

#define NU_TYPE_TREE_MODEL (nu_tree_model_get_type())
#define NU_TREE_MODEL(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
                            NU_TYPE_TREE_MODEL, NUTreeModel))
#define NU_IS_TREE_MODEL(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
                               NU_TYPE_TREE_MODEL))

typedef struct _NUTreeModel        NUTreeModel;
typedef struct _NUTreeModelPrivate NUTreeModelPrivate;
typedef struct _NUTreeModelClass   NUTreeModelClass;

struct _NUTreeModel {
  GObject parent;
  NUTreeModelPrivate* priv;
};

struct _NUTreeModelClass {
  GObjectClass parent_class;
};

GType nu_tree_model_get_type();
NUTreeModel* nu_tree_model_new(Table* table, TableModel* model);

}  // namespace nu

#endif  // NATIVEUI_GTK_NU_TREE_MODEL_H_
