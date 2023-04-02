// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/image.h"

#include <gtk/gtk.h>

namespace nu {

namespace {

// Create an empty image with only 1 frame.
NativeImage CreateEmptyImage() {
  GdkPixbufSimpleAnim* image = gdk_pixbuf_simple_anim_new(1, 1, 1.f);
  GdkPixbuf* frame = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, 1, 1);
  gdk_pixbuf_simple_anim_add_frame(image, frame);
  g_object_unref(frame);
  return GDK_PIXBUF_ANIMATION(image);
}

// Called to free the surface.
void OnPixbufDestroy(guchar* data, gpointer surface) {
  cairo_surface_destroy(static_cast<cairo_surface_t*>(surface));
}

}  // namespace

Image::Image() : image_(CreateEmptyImage()), is_empty_(true) {}

Image::Image(const base::FilePath& p)
    : scale_factor_(GetScaleFactorFromFilePath(p)),
      image_(gdk_pixbuf_animation_new_from_file(p.value().c_str(), nullptr)) {
  // When file reading failed |image_| could be nullptr, having a null
  // native image is very dangerous so we create an empty image when it
  // happens.
  if (!image_) {
    image_ = CreateEmptyImage();
    is_empty_ = true;
  }
}

Image::Image(const Buffer& buffer, float scale_factor)
    : scale_factor_(scale_factor) {
  GInputStream* stream = g_memory_input_stream_new_from_data(
      buffer.content(), buffer.size(), nullptr);
  image_ = gdk_pixbuf_animation_new_from_stream(stream, nullptr, nullptr);
  if (!image_) {
    image_ = CreateEmptyImage();
    is_empty_ = true;
  }
  g_object_unref(stream);
}

Image::~Image() {
  g_object_unref(image_);
  if (iter_)
    g_object_unref(iter_);
}

bool Image::IsEmpty() const {
  return is_empty_;
}

SizeF Image::GetSize() const {
  if (is_empty_)
    return SizeF();
  return ScaleSize(SizeF(gdk_pixbuf_animation_get_width(image_),
                         gdk_pixbuf_animation_get_height(image_)),
                   1.f / scale_factor_);
}

Image* Image::Tint(Color color) const {
  // Create surface to draw.
  int width = gdk_pixbuf_animation_get_width(image_);
  int height = gdk_pixbuf_animation_get_height(image_);
  cairo_surface_t* surface = cairo_image_surface_create(
      CAIRO_FORMAT_ARGB32, width, height);
  cairo_t* context = cairo_create(surface);
  // Draw image on the surface.
  GdkPixbuf* pixbuf = gdk_pixbuf_animation_get_static_image(image_);
  gdk_cairo_set_source_pixbuf(context, pixbuf, 0, 0);
  cairo_paint(context);
  // Apply tint color.
  cairo_set_operator(context, CAIRO_OPERATOR_ATOP);
  cairo_set_source_rgba(context, color.r() / 255., color.g() / 255.,
                                 color.b() / 255., color.a() / 255.);
  cairo_rectangle(context, 0, 0, width, height);
  cairo_fill(context);
  cairo_destroy(context);
  // Save the surface to pixbuf.
  GdkPixbuf* frame = gdk_pixbuf_new_from_data(
      cairo_image_surface_get_data(surface), GDK_COLORSPACE_RGB, TRUE, 8,
      width, height, cairo_image_surface_get_stride(surface),
      // GdkPixbuf does not hold the data, we have to keep the surface alive
      // until the pixbuf is destroyed.
      OnPixbufDestroy, surface);
  // Create new image.
  GdkPixbufSimpleAnim* image = gdk_pixbuf_simple_anim_new(width, height, 1.f);
  gdk_pixbuf_simple_anim_add_frame(image, frame);
  g_object_unref(frame);
  return new Image(GDK_PIXBUF_ANIMATION(image), scale_factor_);
}

bool Image::WriteToFile(const std::string& format,
                        const base::FilePath& target) {
  GdkPixbuf* pixbuf = gdk_pixbuf_animation_get_static_image(image_);
  return gdk_pixbuf_save(pixbuf, target.value().c_str(), format.c_str(),
                         nullptr, nullptr);
}

void Image::AdvanceFrame() {
  GTimeVal time;
  g_get_current_time(&time);
  if (iter_)
    gdk_pixbuf_animation_iter_advance(iter_, &time);
  else
    iter_ = gdk_pixbuf_animation_get_iter(image_, &time);
}

}  // namespace nu
