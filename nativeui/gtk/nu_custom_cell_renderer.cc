// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/nu_custom_cell_renderer.h"

#include "base/values.h"
#include "nativeui/gfx/gtk/painter_gtk.h"

namespace nu {

enum { PROP_VALUE = 1 };

struct _NUCustomCellRendererPrivate {
  Table::ColumnOptions options;
  base::Value value;
};

static void nu_custom_cell_renderer_class_init(
    NUCustomCellRendererClass *klass);
static void nu_custom_cell_renderer_finalize(GObject* gobject);
static void nu_custom_cell_renderer_set_property(GObject* object,
                                                 guint param_id,
                                                 const GValue* value,
                                                 GParamSpec* pspec);
static void nu_custom_cell_renderer_get_size(GtkCellRenderer* renderer,
                                             GtkWidget* widget,
                                             const GdkRectangle* cell_area,
                                             gint* x_offset,
                                             gint* y_offset,
                                             gint* width,
                                             gint* height);
static void nu_custom_cell_renderer_render(GtkCellRenderer* cell,
                                           cairo_t* cr,
                                           GtkWidget* widget,
                                           const GdkRectangle* background_area,
                                           const GdkRectangle* cell_area,
                                           GtkCellRendererState flags);

G_DEFINE_TYPE_WITH_PRIVATE(NUCustomCellRenderer,
                           nu_custom_cell_renderer,
                           GTK_TYPE_CELL_RENDERER)

static void nu_custom_cell_renderer_class_init(NUCustomCellRendererClass* cl) {
  auto* object_class = G_OBJECT_CLASS(cl);
  object_class->finalize = nu_custom_cell_renderer_finalize;
  object_class->set_property = nu_custom_cell_renderer_set_property;

  auto* cell_class = GTK_CELL_RENDERER_CLASS(cl);
  cell_class->get_size = nu_custom_cell_renderer_get_size;
  cell_class->render = nu_custom_cell_renderer_render;

  g_object_class_install_property(object_class,
                                  PROP_VALUE,
                                  g_param_spec_pointer("value",
                                                       "Value",
                                                       "The value to display",
                                                       G_PARAM_WRITABLE));
}

static void nu_custom_cell_renderer_finalize(GObject* object) {
  // Call in-place destructor since we don't manage its memory.
  NUCustomCellRendererPrivate* priv = NU_CUSTOM_CELL_RENDERER(object)->priv;
  priv->options.Table::ColumnOptions::~ColumnOptions();
  priv->value.base::Value::~Value();

  G_OBJECT_CLASS(nu_custom_cell_renderer_parent_class)->finalize(object);
}

static void nu_custom_cell_renderer_set_property(GObject* object,
                                                 guint param_id,
                                                 const GValue* gval,
                                                 GParamSpec* pspec) {
  if (param_id != PROP_VALUE) {
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
    return;
  }
  NUCustomCellRendererPrivate* priv = NU_CUSTOM_CELL_RENDERER(object)->priv;
  auto* value = static_cast<const base::Value*>(g_value_get_pointer(gval));
  if (value)
    priv->value = base::Value(value->Clone());
  else
    priv->value = base::Value();
}

static void nu_custom_cell_renderer_get_size(GtkCellRenderer* renderer,
                                             GtkWidget* widget,
                                             const GdkRectangle* cell_area,
                                             gint* x_offset,
                                             gint* y_offset,
                                             gint* width,
                                             gint* height) {
  if (x_offset)
    *x_offset = 0;
  if (y_offset)
    *y_offset = 0;
  if (width)
    *width = 0;
  if (height)
    *height = 0;
}

static void nu_custom_cell_renderer_render(GtkCellRenderer* cell,
                                           cairo_t* cr,
                                           GtkWidget* widget,
                                           const GdkRectangle* background_area,
                                           const GdkRectangle* cell_area,
                                           GtkCellRendererState flags) {
  NUCustomCellRendererPrivate* priv = NU_CUSTOM_CELL_RENDERER(cell)->priv;
  if (!priv->options.on_draw)
    return;

  cairo_translate(cr, cell_area->x, cell_area->y);
  cairo_rectangle(cr, 0, 0, cell_area->width, cell_area->height);
  cairo_clip(cr);

  PainterGtk painter(cr, SizeF(cell_area->width, cell_area->height));
  priv->options.on_draw(&painter,
                        nu::RectF(0, 0, cell_area->width, cell_area->height),
                        priv->value);
}

static void nu_custom_cell_renderer_init(NUCustomCellRenderer* cell) {
  g_object_set(G_OBJECT(cell), "mode", GTK_CELL_RENDERER_MODE_INERT, nullptr);
  cell->priv = static_cast<NUCustomCellRendererPrivate*>(
      nu_custom_cell_renderer_get_instance_private(cell));
  new(&cell->priv->value) base::Value();
}

GtkCellRenderer* nu_custom_cell_renderer_new(
    const Table::ColumnOptions& options) {
  void* object = g_object_new(NU_TYPE_CUSTOM_CELL_RENDERER, nullptr);
  // Do in-place new since memory has already been allocated.
  NUCustomCellRendererPrivate* priv = NU_CUSTOM_CELL_RENDERER(object)->priv;
  new(&priv->options) Table::ColumnOptions(options);
  return GTK_CELL_RENDERER(object);
}

}  // namespace nu
