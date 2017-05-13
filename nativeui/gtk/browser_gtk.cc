// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

namespace nu {

namespace {

inline WebKitWebView* GetWebView(GtkWidget* widget) {
  return WEBKIT_WEB_VIEW(g_object_get_data(G_OBJECT(widget), "webview"));
}

}  // namespace

Browser::Browser() {
  GtkWidget* webview = webkit_web_view_new();
  gtk_widget_show(webview);
  // Add scrollbar.
  GtkWidget* scroll = gtk_scrolled_window_new(nullptr, nullptr);
  gtk_container_add(GTK_CONTAINER(scroll), webview);
  // Make it easier to receive the webview later.
  g_object_set_data(G_OBJECT(scroll), "webview", webview);
  TakeOverView(scroll);
}

Browser::~Browser() {
}

void Browser::LoadURL(const std::string& url) {
  webkit_web_view_load_uri(GetWebView(GetNative()), url.c_str());
}

}  // namespace nu
