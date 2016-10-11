// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/image.h"

namespace nu {

Image::Image(const String& file)
    : image_(gdk_pixbuf_new_from_file(file.c_str(), nullptr)) {
}

Image::~Image() {
  if (image_)
    g_object_unref(image_);
}

Size Image::GetSize() const {
  if (image_)
    return Size(gdk_pixbuf_get_width(image_), gdk_pixbuf_get_height(image_));
  else
    return Size();
}

}  // namespace nu
