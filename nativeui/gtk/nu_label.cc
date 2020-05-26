// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/nu_label.h"

#include "nativeui/gfx/attributed_text.h"
#include "nativeui/gfx/gtk/painter_gtk.h"
#include "nativeui/label.h"

namespace nu {

struct _NULabelPrivate {
  Label* label;
};

static void nu_label_get_preferred_width(GtkWidget* widget,
                                         gint* minimum,
                                         gint* natural);
static void nu_label_get_preferred_height(GtkWidget* widget,
                                          gint* minimum,
                                          gint* natural);
static gboolean nu_label_draw(GtkWidget* widget, cairo_t* cr);

G_DEFINE_TYPE_WITH_PRIVATE(NULabel, nu_label, GTK_TYPE_WIDGET)

static void nu_label_class_init(NULabelClass* nu_class) {
  GtkWidgetClass* widget_class =
      reinterpret_cast<GtkWidgetClass*>(nu_class);

  widget_class->get_preferred_width = nu_label_get_preferred_width;
  widget_class->get_preferred_height = nu_label_get_preferred_height;
  widget_class->draw = nu_label_draw;
}

static void nu_label_get_preferred_width(GtkWidget* widget,
                                         gint* minimum,
                                         gint* natural) {
  *minimum = *natural = 0;
}

static void nu_label_get_preferred_height(GtkWidget* widget,
                                          gint* minimum,
                                          gint* natural) {
  *minimum = *natural = 0;
}

static gboolean nu_label_draw(GtkWidget* widget, cairo_t* cr) {
  int width = gtk_widget_get_allocated_width(widget);
  int height = gtk_widget_get_allocated_height(widget);
  gtk_render_background(gtk_widget_get_style_context(widget), cr,
                        0, 0, width, height);

  auto* label = NU_LABEL(widget)->priv->label;
  PainterGtk painter(cr, SizeF(width, height));
  painter.DrawAttributedText(label->GetAttributedText(),
                             RectF(0, 0, width, height));
  return false;
}

static void nu_label_init(NULabel* widget) {
  gtk_widget_set_has_window(GTK_WIDGET(widget), FALSE);
  widget->priv = static_cast<NULabelPrivate*>(
      nu_label_get_instance_private(widget));
}

GtkWidget* nu_label_new(Label* label) {
  void* widget = g_object_new(NU_TYPE_LABEL, nullptr);
  NU_LABEL(widget)->priv->label = label;
  return GTK_WIDGET(widget);
}

}  // namespace nu
