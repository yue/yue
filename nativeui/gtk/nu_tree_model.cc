// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/nu_tree_model.h"

#include "nativeui/table.h"
#include "nativeui/table_model.h"

namespace nu {

struct _NUTreeModelPrivate {
  Table* table;
  TableModel* model;
};

static void nu_tree_model_tree_model_init(GtkTreeModelIface* iface);
static void nu_tree_model_finalize(GObject* obj);
static GtkTreeModelFlags nu_tree_model_get_flags(GtkTreeModel* tree_model);
static gint nu_tree_model_get_n_columns(GtkTreeModel* tree_model);
static GType nu_tree_model_get_column_type(GtkTreeModel* tree_model,
                                           gint index);
static gboolean nu_tree_model_get_iter(GtkTreeModel* tree_model,
                                       GtkTreeIter* iter,
                                       GtkTreePath* path);
static GtkTreePath* nu_tree_model_get_path(GtkTreeModel* tree_model,
                                           GtkTreeIter* iter);
static void nu_tree_model_get_value(GtkTreeModel* tree_model,
                                    GtkTreeIter* iter,
                                    gint column,
                                    GValue* value);
static gboolean nu_tree_model_iter_next(GtkTreeModel* tree_model,
                                        GtkTreeIter* iter);
static gboolean nu_tree_model_iter_previous(GtkTreeModel* tree_model,
                                            GtkTreeIter* iter);
static gboolean nu_tree_model_iter_children(GtkTreeModel* tree_model,
                                            GtkTreeIter* iter,
                                            GtkTreeIter* parent);
static gboolean nu_tree_model_iter_has_child(GtkTreeModel* tree_model,
                                             GtkTreeIter* iter);
static gint nu_tree_model_iter_n_children(GtkTreeModel* tree_model,
                                          GtkTreeIter* iter);
static gboolean nu_tree_model_iter_nth_child(GtkTreeModel* tree_model,
                                             GtkTreeIter* iter,
                                             GtkTreeIter* parent,
                                             gint n);
static gboolean nu_tree_model_iter_parent(GtkTreeModel* tree_model,
                                          GtkTreeIter* iter,
                                          GtkTreeIter* child);

G_DEFINE_TYPE_WITH_CODE(NUTreeModel, nu_tree_model, G_TYPE_OBJECT,
                        G_ADD_PRIVATE(NUTreeModel)
                        G_IMPLEMENT_INTERFACE(GTK_TYPE_TREE_MODEL,
                                              nu_tree_model_tree_model_init))

static void nu_tree_model_class_init(NUTreeModelClass* cl) {
  G_OBJECT_CLASS(cl)->finalize = nu_tree_model_finalize;
}

static void nu_tree_model_tree_model_init(GtkTreeModelIface* iface) {
  iface->get_flags = nu_tree_model_get_flags;
  iface->get_n_columns = nu_tree_model_get_n_columns;
  iface->get_column_type = nu_tree_model_get_column_type;
  iface->get_iter = nu_tree_model_get_iter;
  iface->get_path = nu_tree_model_get_path;
  iface->get_value = nu_tree_model_get_value;
  iface->iter_next = nu_tree_model_iter_next;
  iface->iter_previous = nu_tree_model_iter_previous;
  iface->iter_children = nu_tree_model_iter_children;
  iface->iter_has_child = nu_tree_model_iter_has_child;
  iface->iter_n_children = nu_tree_model_iter_n_children;
  iface->iter_nth_child = nu_tree_model_iter_nth_child;
  iface->iter_parent = nu_tree_model_iter_parent;
}

static void nu_tree_model_finalize(GObject* obj) {
  G_OBJECT_CLASS(nu_tree_model_parent_class)->finalize(obj);
}

static GtkTreeModelFlags nu_tree_model_get_flags(GtkTreeModel* tree_model) {
  return GTK_TREE_MODEL_LIST_ONLY;
}

static gint nu_tree_model_get_n_columns(GtkTreeModel* tree_model) {
  NUTreeModelPrivate* priv = NU_TREE_MODEL(tree_model)->priv;
  return priv->table->GetColumnCount();
}

static GType nu_tree_model_get_column_type(GtkTreeModel* tree_model,
                                           gint index) {
  return G_TYPE_POINTER;
}

static gboolean nu_tree_model_get_iter(GtkTreeModel* tree_model,
                                       GtkTreeIter* iter,
                                       GtkTreePath* path) {
  NUTreeModelPrivate* priv = NU_TREE_MODEL(tree_model)->priv;
  iter->stamp = false;
  if (gtk_tree_path_get_depth(path) != 1)
    return false;
  gint row = gtk_tree_path_get_indices(path)[0];
  if (row < 0 || static_cast<uint32_t>(row) >= priv->model->GetRowCount())
    return false;
  iter->stamp = true;
  iter->user_data = GINT_TO_POINTER(row);
  return true;
}

static GtkTreePath* nu_tree_model_get_path(GtkTreeModel* tree_model,
                                           GtkTreeIter* iter) {
  if (!iter->stamp)
    return nullptr;
  gint row = GPOINTER_TO_INT(iter->user_data);
  return gtk_tree_path_new_from_indices(row, -1);
}

static void nu_tree_model_get_value(GtkTreeModel* tree_model,
                                    GtkTreeIter* iter,
                                    gint column,
                                    GValue* value) {
  if (!iter->stamp)
    return;
  NUTreeModelPrivate* priv = NU_TREE_MODEL(tree_model)->priv;
  gint row = GPOINTER_TO_INT(iter->user_data);
  g_value_init(value, G_TYPE_POINTER);
  g_value_set_pointer(
      value, const_cast<base::Value*>(priv->model->GetValue(column, row)));
}

static gboolean nu_tree_model_iter_next(GtkTreeModel* tree_model,
                                        GtkTreeIter* iter) {
  if (!iter->stamp)
    return false;
  NUTreeModelPrivate* priv = NU_TREE_MODEL(tree_model)->priv;
  gint row = GPOINTER_TO_INT(iter->user_data);
  if (row + 1 >= static_cast<int>(priv->model->GetRowCount())) {
    iter->stamp = false;
    return false;
  }
  iter->stamp = true;
  iter->user_data = GINT_TO_POINTER(row + 1);
  return true;
}

static gboolean nu_tree_model_iter_previous(GtkTreeModel* tree_model,
                                            GtkTreeIter* iter) {
  if (!iter->stamp)
    return false;
  gint row = GPOINTER_TO_INT(iter->user_data);
  if (row - 1 < 0) {
    iter->stamp = false;
    return false;
  }
  iter->stamp = true;
  iter->user_data = GINT_TO_POINTER(row - 1);
  return true;
}

static gboolean nu_tree_model_iter_children(GtkTreeModel* tree_model,
                                            GtkTreeIter* iter,
                                            GtkTreeIter* parent) {
  return gtk_tree_model_iter_nth_child(tree_model, iter, parent, 0);
}

static gboolean nu_tree_model_iter_has_child(GtkTreeModel* tree_model,
                                             GtkTreeIter* iter) {
  return false;
}

static gint nu_tree_model_iter_n_children(GtkTreeModel* tree_model,
                                          GtkTreeIter* iter) {
  NUTreeModelPrivate* priv = NU_TREE_MODEL(tree_model)->priv;
  return iter ? 0 : priv->table->GetColumnCount();
}

static gboolean nu_tree_model_iter_nth_child(GtkTreeModel* tree_model,
                                             GtkTreeIter* iter,
                                             GtkTreeIter* parent,
                                             gint n) {
  iter->stamp = false;
  if (parent)
    return false;
  NUTreeModelPrivate* priv = NU_TREE_MODEL(tree_model)->priv;
  if (n < 0 || static_cast<uint32_t>(n) >= priv->model->GetRowCount())
    return false;
  iter->stamp = true;
  iter->user_data = GINT_TO_POINTER(n);
  return true;
}

static gboolean nu_tree_model_iter_parent(GtkTreeModel* tree_model,
                                          GtkTreeIter* iter,
                                          GtkTreeIter* child) {
  iter->stamp = false;
  return false;
}

static void nu_tree_model_init(NUTreeModel* tree_model) {
  tree_model->priv = static_cast<NUTreeModelPrivate*>(
      nu_tree_model_get_instance_private(tree_model));
}

NUTreeModel* nu_tree_model_new(Table* table, TableModel* model) {
  void* obj = g_object_new(NU_TYPE_TREE_MODEL, nullptr);
  NU_TREE_MODEL(obj)->priv->table = table;
  NU_TREE_MODEL(obj)->priv->model = model;
  return NU_TREE_MODEL(obj);
}

}  // namespace nu
