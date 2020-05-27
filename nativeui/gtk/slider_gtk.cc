// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/slider.h"

#include <gtk/gtk.h>

#include "nativeui/gtk/util/widget_util.h"

namespace nu {

namespace {

void OnValueChanged(GObject* widget, Slider* slider) {
  if (g_object_get_data(widget, "ignore-value-change")) {
    g_object_set_data(widget, "ignore-value-change", nullptr);
    return;
  }
  slider->on_value_change.Emit(slider);
}

gboolean OnButtonRelease(GtkRange*, GdkEventButton* event, Slider* slider) {
  if (event->type == GDK_BUTTON_RELEASE)
    slider->on_sliding_complete.Emit(slider);
  return false;
}

}  // namespace

NativeView Slider::PlatformCreate() {
  GtkWidget* scale = gtk_scale_new_with_range(
      GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
  gtk_scale_set_draw_value(GTK_SCALE(scale), false);
  g_signal_connect(scale, "value-changed", G_CALLBACK(OnValueChanged), this);
  g_signal_connect(scale, "button-release-event",
                   G_CALLBACK(OnButtonRelease), this);
  return scale;
}

void Slider::SetValue(float value) {
  g_object_set_data(G_OBJECT(GetNative()), "ignore-value-change", this);
  gtk_range_set_value(GTK_RANGE(GetNative()), value);
}

float Slider::GetValue() const {
  return gtk_range_get_value(GTK_RANGE(GetNative()));
}

void Slider::SetStep(float step) {
  gtk_range_set_increments(GTK_RANGE(GetNative()), step, step);
}

float Slider::GetStep() const {
  GtkAdjustment* adjustment = gtk_range_get_adjustment(GTK_RANGE(GetNative()));
  return gtk_adjustment_get_step_increment(adjustment);
}

void Slider::SetRange(float min, float max) {
  gtk_range_set_range(GTK_RANGE(GetNative()), min, max);
}

std::tuple<float, float> Slider::GetRange() const {
  GtkAdjustment* adjustment = gtk_range_get_adjustment(GTK_RANGE(GetNative()));
  return std::make_tuple(gtk_adjustment_get_lower(adjustment),
                         gtk_adjustment_get_upper(adjustment));
}

SizeF Slider::GetMinimumSize() const {
  return GetPreferredSizeForWidget(GetNative());
}

}  // namespace nu
