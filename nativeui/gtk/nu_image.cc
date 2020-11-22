// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/nu_image.h"

#include "nativeui/gfx/gtk/painter_gtk.h"
#include "nativeui/gfx/image.h"

namespace nu {

struct _NUImagePrivate {
  Image* image;
  Size size;
};

static void nu_image_get_preferred_width(GtkWidget* widget,
                                         gint* minimum,
                                         gint* natural);
static void nu_image_get_preferred_height(GtkWidget* widget,
                                          gint* minimum,
                                          gint* natural);
static gboolean nu_image_draw(GtkWidget* widget, cairo_t* cr);

G_DEFINE_TYPE_WITH_PRIVATE(NUImage, nu_image, GTK_TYPE_WIDGET)

static void nu_image_class_init(NUImageClass* nu_class) {
  GtkWidgetClass* widget_class =
      reinterpret_cast<GtkWidgetClass*>(nu_class);

  widget_class->get_preferred_width = nu_image_get_preferred_width;
  widget_class->get_preferred_height = nu_image_get_preferred_height;
  widget_class->draw = nu_image_draw;
}

static void nu_image_get_preferred_width(GtkWidget* widget,
                                         gint* minimum,
                                         gint* natural) {
  NUImagePrivate* priv = NU_IMAGE(widget)->priv;
  int width = priv->size.IsEmpty() ? priv->image->GetSize().width()
                                   : priv->size.width();
  *minimum = *natural = width;
}

static void nu_image_get_preferred_height(GtkWidget* widget,
                                          gint* minimum,
                                          gint* natural) {
  NUImagePrivate* priv = NU_IMAGE(widget)->priv;
  int height = priv->size.IsEmpty() ? priv->image->GetSize().height()
                                    : priv->size.height();
  *minimum = *natural = height;
}

static gboolean nu_image_draw(GtkWidget* widget, cairo_t* cr) {
  NUImagePrivate* priv = NU_IMAGE(widget)->priv;

  int width = gtk_widget_get_allocated_width(widget);
  int height = gtk_widget_get_allocated_height(widget);
  PainterGtk painter(cr, SizeF(width, height));
  painter.DrawImage(priv->image, RectF(0, 0, width, height));
  return false;
}

static void nu_image_init(NUImage* widget) {
  gtk_widget_set_has_window(GTK_WIDGET(widget), FALSE);
  widget->priv = static_cast<NUImagePrivate*>(
      nu_image_get_instance_private(widget));
}

GtkWidget* nu_image_new(Image* image) {
  void* widget = g_object_new(NU_TYPE_IMAGE, nullptr);
  NU_IMAGE(widget)->priv->image = image;
  return GTK_WIDGET(widget);
}

void nu_image_set_size(NUImage* widget, const Size& size) {
  widget->priv->size = size;
}

}  // namespace nu
