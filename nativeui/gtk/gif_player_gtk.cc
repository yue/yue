// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gif_player.h"

#include <gtk/gtk.h>

#include "nativeui/gfx/gtk/painter_gtk.h"
#include "nativeui/gfx/image.h"

namespace nu {

namespace {

// Determine whether the image can be animated.
bool CanImageAnimate(Image* image) {
  if (!image)
    return false;
  return !gdk_pixbuf_animation_is_static_image(image->GetNative());
}

// Callback for drawing GifPlayer.
gboolean OnDraw(GtkWidget* widget, cairo_t* cr, GifPlayer* view) {
  Image* image = view->GetImage();
  if (!image)
    return TRUE;

  // Default background.
  int width = gtk_widget_get_allocated_width(widget);
  int height = gtk_widget_get_allocated_height(widget);
  gtk_render_background(gtk_widget_get_style_context(widget), cr,
                        0, 0, width, height);

  // Calulate image position.
  GdkPixbuf* pixbuf = CanImageAnimate(image) ?
      gdk_pixbuf_animation_iter_get_pixbuf(view->GetFrame()) :
      gdk_pixbuf_animation_get_static_image(image->GetNative());
  int image_width = gdk_pixbuf_get_width(pixbuf);
  int image_height = gdk_pixbuf_get_height(pixbuf);
  cairo_translate(cr, (width - image_width) / 2, (height - image_height) / 2);

  // Paint.
  gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
  cairo_paint(cr);
  return FALSE;
}

}  // namespace

GifPlayer::GifPlayer() {
  TakeOverView(gtk_drawing_area_new());
  g_signal_connect(GetNative(), "draw", G_CALLBACK(OnDraw), this);
}

GifPlayer::~GifPlayer() {
  if (iter_)
    g_object_unref(iter_);
  if (timer_ > 0)
    g_source_remove(timer_);
}

void GifPlayer::PlatformSetImage(Image* image) {
  gtk_widget_queue_draw(GetNative());
  // Reset iterator after changing image.
  if (iter_) {
    g_object_unref(iter_);
    iter_ = nullptr;
  }
  // Start animation by default.
  SetAnimating(!!image);
}

void GifPlayer::SetAnimating(bool animates) {
  // Reset timer.
  if (timer_ > 0) {
    g_source_remove(timer_);
    timer_ = 0;
  }
  // Do not animate static image.
  if (!CanImageAnimate(image_.get())) {
    is_animating_ = false;
    return;
  }
  is_animating_ = animates;
  // Create a timer to play animation.
  if (is_animating_)
    ScheduleFrame(this);
}

bool GifPlayer::IsAnimating() const {
  return is_animating_;
}

GdkPixbufAnimationIter* GifPlayer::GetFrame() {
  if (!iter_) {
    GTimeVal time;
    g_get_current_time(&time);
    iter_ = gdk_pixbuf_animation_get_iter(image_->GetNative(), &time);
  }
  return iter_;
}

gboolean GifPlayer::ScheduleFrame(GifPlayer* self) {
  if (self->iter_) {
    // Advance frame.
    GTimeVal time;
    g_get_current_time(&time);
    gdk_pixbuf_animation_iter_advance(self->iter_, &time);
  } else {
    self->GetFrame();
  }
  // Emit draw event.
  gtk_widget_queue_draw(self->GetNative());
  // Schedule next call.
  if (self->is_animating_) {
    g_timeout_add(gdk_pixbuf_animation_iter_get_delay_time(self->iter_),
                  reinterpret_cast<GSourceFunc>(&GifPlayer::ScheduleFrame),
                  self);
  }
  return FALSE;
}

}  // namespace nu
