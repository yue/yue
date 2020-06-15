// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gif_player.h"

#include <gtk/gtk.h>

#include "nativeui/gfx/gtk/painter_gtk.h"
#include "nativeui/gfx/image.h"

namespace nu {

namespace {

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

  PainterGtk painter(cr, SizeF(width, height));
  view->Paint(&painter);
  return FALSE;
}

// Callback when widget is show.
void OnShow(GtkWidget* widget, GifPlayer* view) {
  if (view->IsAnimating() && !view->IsPlaying())
    view->ScheduleFrame();
}

// Callback when widget is hidden.
void OnHide(GtkWidget* widget, GifPlayer* view) {
  if (view->IsAnimating())
    view->StopAnimationTimer();
}

}  // namespace

GifPlayer::GifPlayer() {
  TakeOverView(gtk_drawing_area_new());
  g_signal_connect(GetNative(), "draw", G_CALLBACK(OnDraw), this);
  g_signal_connect(GetNative(), "show", G_CALLBACK(OnShow), this);
  g_signal_connect(GetNative(), "hide", G_CALLBACK(OnHide), this);
}

GifPlayer::~GifPlayer() {
  if (timer_ > 0)
    StopAnimationTimer();
}

void GifPlayer::PlatformSetImage(Image* image) {
  SchedulePaint();
  // Start animation by default.
  SetAnimating(!!image);
}

bool GifPlayer::CanAnimate() const {
  return image_ && !gdk_pixbuf_animation_is_static_image(image_->GetNative());
}

void GifPlayer::ScheduleFrame() {
  // Advance frame.
  image_->AdvanceFrame();
  // Emit draw event.
  SchedulePaint();
  // Schedule next call.
  if (is_animating_) {
    timer_ = MessageLoop::SetTimeout(
        gdk_pixbuf_animation_iter_get_delay_time(image_->iter()),
        std::bind(&GifPlayer::ScheduleFrame, this));
  }
}

}  // namespace nu
