// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gtk/nu_container.h"

#include "nativeui/container.h"
#include "nativeui/gfx/gtk/painter_gtk.h"

namespace nu {

struct _NUContainerPrivate {
  Container* delegate;
  GdkWindow* event_window;
  gint event_mask;
};

namespace {

// Create invisible input window.
void CreateEventWindow(GtkWidget* widget, NUContainerPrivate* priv) {
  GtkAllocation allocation;
  gtk_widget_get_allocation(widget, &allocation);
  GdkWindowAttr attributes;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = allocation.x;
  attributes.y = allocation.x;
  attributes.width = allocation.width;
  attributes.height = allocation.height;
  attributes.wclass = GDK_INPUT_ONLY;
  attributes.event_mask = gtk_widget_get_events(widget) | priv->event_mask;
  priv->event_window = gdk_window_new(gtk_widget_get_parent_window(widget),
                                      &attributes, GDK_WA_X | GDK_WA_Y);
  gtk_widget_register_window(widget, priv->event_window);
  gdk_window_move_resize(priv->event_window,
                         allocation.x, allocation.y,
                         allocation.width, allocation.height);
}

void DestroyEventWindow(GtkWidget* widget, NUContainerPrivate* priv) {
  if (!priv->event_window)
    return;
  gtk_widget_unregister_window(widget, priv->event_window);
  gdk_window_destroy(priv->event_window);
  priv->event_window = nullptr;
}

}  // namespace

static void nu_container_realize(GtkWidget* widget);
static void nu_container_unrealize(GtkWidget* widget);
static void nu_container_map(GtkWidget* widget);
static void nu_container_unmap(GtkWidget* widget);
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
  widget_class->unrealize = nu_container_unrealize;
  widget_class->map = nu_container_map;
  widget_class->unmap = nu_container_unmap;
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
  CreateEventWindow(widget, NU_CONTAINER(widget)->priv);
}

static void nu_container_unrealize(GtkWidget* widget) {
  DestroyEventWindow(widget, NU_CONTAINER(widget)->priv);
  GTK_WIDGET_CLASS(nu_container_parent_class)->unrealize(widget);
}

static void nu_container_map(GtkWidget* widget) {
  NUContainerPrivate* priv = NU_CONTAINER(widget)->priv;
  if (priv->event_window)
    gdk_window_show(priv->event_window);

  GTK_WIDGET_CLASS(nu_container_parent_class)->map(widget);
}

static void nu_container_unmap(GtkWidget* widget) {
  NUContainerPrivate* priv = NU_CONTAINER(widget)->priv;
  if (priv->event_window)
    gdk_window_hide(priv->event_window);

  GTK_WIDGET_CLASS(nu_container_parent_class)->unmap(widget);
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

  // Note that we don't use nu::Container::OnSizeChanged for updating layout,
  // because GTK requires us do ing size allocation for children here otherwise
  // native widgets will have problems rendering.
  NUContainerPrivate* priv = NU_CONTAINER(widget)->priv;
  priv->delegate->UpdateChildBounds();

  if (gtk_widget_get_realized(widget) && priv->event_window) {
    gdk_window_move_resize(priv->event_window,
                           allocation->x, allocation->y,
                           allocation->width, allocation->height);
  }
}

static void nu_container_style_updated(GtkWidget* widget) {
  GTK_WIDGET_CLASS(nu_container_parent_class)->style_updated(widget);
}

static gboolean nu_container_draw(GtkWidget* widget, cairo_t* cr) {
  int width = gtk_widget_get_allocated_width(widget);
  int height = gtk_widget_get_allocated_height(widget);
  gtk_render_background(gtk_widget_get_style_context(widget), cr,
                        0, 0, width, height);

  Container* delegate = NU_CONTAINER(widget)->priv->delegate;
  PainterGtk painter(cr, SizeF(width, height));
  delegate->on_draw.Emit(delegate, &painter, nu::RectF(0, 0, width, height));

  for (int i = 0; i < delegate->ChildCount(); ++i)
    gtk_container_propagate_draw(GTK_CONTAINER(widget),
                                 delegate->ChildAt(i)->GetNative(), cr);
  return FALSE;
}

static void nu_container_add(GtkContainer* container, GtkWidget* widget) {
  gtk_widget_set_parent(widget, GTK_WIDGET(container));
}

static void nu_container_remove(GtkContainer* container, GtkWidget* widget) {
  gtk_widget_unparent(widget);
}

static void nu_container_forall(GtkContainer* widget,
                                gboolean include_internals,
                                GtkCallback callback,
                                gpointer callback_data) {
  Container* delegate = NU_CONTAINER(widget)->priv->delegate;
  for (int i = 0; i < delegate->ChildCount(); ++i)
    (*callback)(delegate->ChildAt(i)->GetNative(), callback_data);
}

static GType nu_container_child_type(GtkContainer* container) {
  return GTK_TYPE_WIDGET;
}

static void nu_container_init(NUContainer* widget) {
  gtk_widget_set_has_window(GTK_WIDGET(widget), FALSE);
  widget->priv = static_cast<NUContainerPrivate*>(
      nu_container_get_instance_private(widget));
}

GtkWidget* nu_container_new(Container* delegate) {
  void* widget = g_object_new(NU_TYPE_CONTAINER, nullptr);
  auto* priv = NU_CONTAINER(widget)->priv;
  priv->delegate = delegate;
  priv->event_window = nullptr;
  priv->event_mask = 0;
  return GTK_WIDGET(widget);
}

GdkWindow* nu_container_get_window(NUContainer* widget) {
  return widget->priv->event_window;
}

void nu_container_add_event_mask(NUContainer* container, gint event_mask) {
  GtkWidget* widget = GTK_WIDGET(container);
  gint new_event_mask = event_mask | container->priv->event_mask;
  if (new_event_mask == container->priv->event_mask)
    return;
  container->priv->event_mask = new_event_mask;
  DestroyEventWindow(widget, container->priv);
  if (gtk_widget_get_realized(widget))
    CreateEventWindow(widget, container->priv);
}

}  // namespace nu
