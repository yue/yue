// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/container_container.h"

#include "nativeui/container.h"
#include "nativeui/gfx/gtk/painter_gtk.h"

namespace nu {

static void nu_container_realize(GtkWidget* widget);
static void nu_container_get_preferred_width(GtkWidget* widget,
                                             gint* minimum,
                                             gint* natural);
static void nu_container_get_preferred_height(GtkWidget* widget,
                                              gint* minimum,
                                              gint* natural);
static void nu_container_size_allocate(GtkWidget* widget,
                                       GtkAllocation* allocation);
static void nu_container_style_updated(GtkWidget* widget);
static gboolean nu_container_draw(GtkWidget* widget, cairo_t* cr);
static void nu_container_add(GtkContainer* container, GtkWidget* widget);
static void nu_container_remove(GtkContainer* container, GtkWidget* widget);
static void nu_container_forall(GtkContainer* container,
                                gboolean include_internals,
                                GtkCallback callback,
                                gpointer callback_data);
static GType nu_container_child_type(GtkContainer* container);

G_DEFINE_TYPE_WITH_PRIVATE(NUContainer, nu_container, GTK_TYPE_CONTAINER)

static void nu_container_class_init(NUContainerClass* nu_class) {
  GtkWidgetClass* widget_class =
      reinterpret_cast<GtkWidgetClass*>(nu_class);
  GtkContainerClass* container_class =
      reinterpret_cast<GtkContainerClass*>(nu_class);

  widget_class->realize = nu_container_realize;
  widget_class->get_preferred_width = nu_container_get_preferred_width;
  widget_class->get_preferred_height = nu_container_get_preferred_height;
  widget_class->size_allocate = nu_container_size_allocate;
  widget_class->draw = nu_container_draw;
  widget_class->style_updated = nu_container_style_updated;

  container_class->add = nu_container_add;
  container_class->remove = nu_container_remove;
  container_class->forall = nu_container_forall;
  container_class->child_type = nu_container_child_type;
  gtk_container_class_handle_border_width(container_class);
}

static void nu_container_realize(GtkWidget* widget) {
  GTK_WIDGET_CLASS(nu_container_parent_class)->realize(widget);
}

static void nu_container_get_preferred_width(GtkWidget* widget,
                                             gint* minimum,
                                             gint* natural) {
  // We are not using GTK's layout system, so just return 0.
  *minimum = 0;
  *natural = 0;
}

static void nu_container_get_preferred_height(GtkWidget* widget,
                                              gint* minimum,
                                              gint* natural) {
  // We are not using GTK's layout system, so just return 0.
  *minimum = 0;
  *natural = 0;
}

static void nu_container_size_allocate(GtkWidget* widget,
                                       GtkAllocation* allocation) {
  gtk_widget_set_allocation(widget, allocation);

  // Ignore empty sizes on initialization.
  if (allocation->x == -1 && allocation->y == -1 &&
      allocation->width == 1 && allocation->height == 1)
    return;

  Container* delegate = static_cast<Container*>(
      g_object_get_data(G_OBJECT(widget), "delegate"));
  delegate->BoundsChanged();
}

static void nu_container_style_updated(GtkWidget* widget) {
  GTK_WIDGET_CLASS(nu_container_parent_class)->style_updated(widget);
}

static gboolean nu_container_draw(GtkWidget* widget, cairo_t* cr) {
  int width = gtk_widget_get_allocated_width(widget);
  int height = gtk_widget_get_allocated_height(widget);
  gtk_render_background(gtk_widget_get_style_context(widget), cr,
                        0, 0, width, height);

  Container* delegate = static_cast<Container*>(
      g_object_get_data(G_OBJECT(widget), "delegate"));
  PainterGtk painter(cr);
  delegate->on_draw.Emit(delegate, &painter, nu::RectF(0, 0, width, height));

  for (int i = 0; i < delegate->ChildCount(); ++i)
    gtk_container_propagate_draw(GTK_CONTAINER(widget),
                                 delegate->ChildAt(i)->view(), cr);
  return FALSE;
}

static void nu_container_add(GtkContainer* container, GtkWidget* widget) {
  gtk_widget_set_parent(widget, GTK_WIDGET(container));
}

static void nu_container_remove(GtkContainer* container, GtkWidget* widget) {
  gtk_widget_unparent(widget);
}

static void nu_container_forall(GtkContainer* container,
                                gboolean include_internals,
                                GtkCallback callback,
                                gpointer callback_data) {
  Container* delegate = static_cast<Container*>(
      g_object_get_data(G_OBJECT(container), "delegate"));
  for (int i = 0; i < delegate->ChildCount(); ++i)
    (*callback)(delegate->ChildAt(i)->view(), callback_data);
}

static GType nu_container_child_type(GtkContainer* container) {
  return GTK_TYPE_WIDGET;
}

static void nu_container_init(NUContainer* widget) {
  gtk_widget_set_has_window(GTK_WIDGET(widget), FALSE);
}

GtkWidget* nu_container_new(Container* delegate) {
  void* widget = g_object_new(NU_TYPE_CONTAINER, NULL);
  g_object_set_data_full(G_OBJECT(widget), "delegate", delegate, nullptr);
  return GTK_WIDGET(widget);
}

}  // namespace nu
