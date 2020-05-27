// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/progress_bar.h"

#include <gtk/gtk.h>

#include "nativeui/gtk/util/widget_util.h"

namespace nu {

namespace {

void DeleteTimer(void* data) {
  guint timer = *reinterpret_cast<guint*>(&data);
  g_source_remove(timer);
}

gboolean OnTimer(GtkWidget* widget) {
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(widget));
  return G_SOURCE_CONTINUE;
}

}  // namespace

ProgressBar::ProgressBar() {
  TakeOverView(gtk_progress_bar_new());
  UpdateDefaultStyle();
}

ProgressBar::~ProgressBar() {
}

void ProgressBar::SetValue(float value) {
  SetIndeterminate(false);
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(GetNative()), value / 100.);
}

float ProgressBar::GetValue() const {
  return gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(GetNative())) * 100.;
}

void ProgressBar::SetIndeterminate(bool indeterminate) {
  bool is_indeterminate = IsIndeterminate();
  if (indeterminate == is_indeterminate)
    return;
  if (indeterminate) {
    guint timer = g_timeout_add(100, reinterpret_cast<GSourceFunc>(OnTimer),
                                GetNative());
    g_object_set_data_full(G_OBJECT(GetNative()), "timer",
                           reinterpret_cast<void*>(timer), DeleteTimer);
  } else {
    g_object_set_data(G_OBJECT(GetNative()), "timer", nullptr);
  }
}

bool ProgressBar::IsIndeterminate() const {
  return g_object_get_data(G_OBJECT(GetNative()), "timer");
}

SizeF ProgressBar::GetMinimumSize() const {
  return GetPreferredSizeForWidget(GetNative());
}

}  // namespace nu
