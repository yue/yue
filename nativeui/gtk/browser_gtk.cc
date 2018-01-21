// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

namespace nu {

namespace {

void OnClose(WebKitWebView* widget, Browser* view) {
  view->on_close.Emit(view);
}

void OnLoadChanged(WebKitWebView* widget,
                   WebKitLoadEvent event,
                   Browser* view) {
  switch (event) {
    case WEBKIT_LOAD_STARTED:
      break;
    case WEBKIT_LOAD_REDIRECTED:
      break;
    case WEBKIT_LOAD_COMMITTED:
      break;
    case WEBKIT_LOAD_FINISHED:
      view->on_finish_navigation.Emit(view);
      break;
  }
}

}  // namespace

Browser::Browser() {
  GtkWidget* webview = webkit_web_view_new();
  TakeOverView(webview);

  // Install events.
  g_signal_connect(webview, "close", G_CALLBACK(OnClose), this);
  g_signal_connect(webview, "load-changed", G_CALLBACK(OnLoadChanged), this);
}

Browser::~Browser() {
}

void Browser::LoadURL(const std::string& url) {
  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(GetNative()), url.c_str());
}

}  // namespace nu
