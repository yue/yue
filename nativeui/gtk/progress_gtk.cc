// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/progress.h"

#include "base/timer/timer.h"
#include "nativeui/gtk/widget_util.h"

namespace nu {

namespace {

void DeleteTimer(void* timer) {
  delete static_cast<base::Timer*>(timer);
}

void OnTimer(Progress* progress) {
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progress->view()));
}

}  // namespace

Progress::Progress() {
  TakeOverView(gtk_progress_bar_new());
  SetDefaultStyle(SizeF(GetPreferredSizeForWidget(view())));
}

Progress::~Progress() {
}

void Progress::SetValue(int value) {
  SetIndeterminate(false);
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(view()), value / 100.0);
}

int Progress::GetValue() const {
  return gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(view())) * 100;
}

void Progress::SetIndeterminate(bool indeterminate) {
  bool is_indeterminate = IsIndeterminate();
  if (indeterminate == is_indeterminate)
    return;
  if (indeterminate) {
    auto* timer = new base::Timer(FROM_HERE,
                                  base::TimeDelta::FromMilliseconds(100),
                                  base::Bind(OnTimer, base::Unretained(this)),
                                  true);
    timer->Reset();
    g_object_set_data_full(G_OBJECT(view()), "timer", timer, DeleteTimer);
  } else {
    g_object_set_data(G_OBJECT(view()), "timer", nullptr);
  }
}

bool Progress::IsIndeterminate() const {
  return g_object_get_data(G_OBJECT(view()), "timer");
}

}  // namespace nu
