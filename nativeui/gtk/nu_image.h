// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GTK_NU_IMAGE_H_
#define NATIVEUI_GTK_NU_IMAGE_H_

#include <gtk/gtk.h>

// Custom widget to show images, with high DPI supported.

namespace nu {

class Image;
class Size;

#define NU_TYPE_IMAGE (nu_image_get_type ())
#define NU_IMAGE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
                       NU_TYPE_IMAGE, NUImage))

typedef struct _NUImage        NUImage;
typedef struct _NUImagePrivate NUImagePrivate;
typedef struct _NUImageClass   NUImageClass;

struct _NUImage {
  GtkWidget container;
  NUImagePrivate* priv;
};

struct _NUImageClass {
  GtkWidgetClass parent_class;
};

GType nu_image_get_type();
GtkWidget* nu_image_new(Image* image);
void nu_image_set_size(NUImage* widget, const Size& size);

}  // namespace nu

#endif  // NATIVEUI_GTK_NU_IMAGE_H_
