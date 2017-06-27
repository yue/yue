// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/lifetime.h"

#include <gtk/gtk.h>

namespace nu {

namespace {

gboolean OnSource(base::Closure* func) {
  func->Run();
  return G_SOURCE_REMOVE;
}

}  // namespace

void Lifetime::PlatformInit() {
  gtk_init(nullptr, nullptr);
}

void Lifetime::PlatformDestroy() {
}

void Lifetime::Run() {
  gtk_main();
}

void Lifetime::Quit() {
  gtk_main_quit();
}

void Lifetime::PostTask(const base::Closure& task) {
  g_idle_add_full(G_PRIORITY_DEFAULT, reinterpret_cast<GSourceFunc>(OnSource),
                  new base::Closure(task), operator delete);
}

void Lifetime::PostDelayedTask(int ms, const base::Closure& task) {
  g_timeout_add_full(G_PRIORITY_DEFAULT, ms,
                     reinterpret_cast<GSourceFunc>(OnSource),
                     new base::Closure(task), operator delete);
}

}  // namespace nu
