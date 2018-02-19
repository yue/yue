// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_NU_CONTAINER_H_
#define NATIVEUI_GTK_NU_CONTAINER_H_

#include <gtk/gtk.h>

// Custom GTK container type for nu::Container.

namespace nu {

class Container;

#define NU_TYPE_CONTAINER (nu_container_get_type())
#define NU_CONTAINER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
                           NU_TYPE_CONTAINER, NUContainer))
#define NU_IS_CONTAINER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
                              NU_TYPE_CONTAINER))

typedef struct _NUContainer        NUContainer;
typedef struct _NUContainerPrivate NUContainerPrivate;
typedef struct _NUContainerClass   NUContainerClass;

struct _NUContainer {
  GtkContainer container;
  NUContainerPrivate* priv;
};

struct _NUContainerClass {
  GtkContainerClass parent_class;
};

GType nu_container_get_type();
GtkWidget* nu_container_new(Container* delegate);
GdkWindow* nu_container_get_window(NUContainer* widget);

}  // namespace nu

#endif  // NATIVEUI_GTK_NU_CONTAINER_H_
