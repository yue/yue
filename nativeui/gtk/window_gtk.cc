// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/window.h"

#include <gtk/gtk.h>

#include "nativeui/gtk/util/widget_util.h"
#include "nativeui/menu_bar.h"

namespace nu {

namespace {

// Window private data.
struct NUWindowPrivate {
  Window* delegate = nullptr;
  bool is_popup = false;
  bool is_csd = false;
  // Cached window bounds, which is the content area including menu bar.
  RectF bounds;
  // Cached window states.
  int window_state = 0;
  // Insets of native window frame.
  InsetsF native_frame;
  // Insets of client shadow.
  InsetsF client_shadow;
  // Min/max sizes.
  bool needs_to_update_minmax_size = false;
  bool use_content_minmax_size = false;
  SizeF min_size;
  SizeF max_size;
  // Mouse capture.
  GdkDevice* captured_device = nullptr;
  // Input shape fields.
  bool is_input_shape_set = false;
  bool is_draw_handler_set = false;
  guint draw_handler_id = 0;
};

// Helper to receive private data.
inline NUWindowPrivate* GetPrivate(const Window* window) {
  return static_cast<NUWindowPrivate*>(g_object_get_data(
      G_OBJECT(window->GetNative()), "private"));
}

// User clicks the close button.
gboolean OnDelete(GtkWidget* widget, GdkEvent* event, Window* window) {
  if (!window->should_close || window->should_close(window))
    window->Close();

  // We are destroying the window ourselves, so prevent the default behavior.
  return TRUE;
}

// Window is mapped.
gboolean OnMap(GtkWidget* widget, GdkEvent* event, NUWindowPrivate* priv) {
  // Calculate the native window frame.
  GetNativeFrameInsets(widget, &priv->native_frame);
  // Calculate the client shadow for CSD window.
  if (priv->is_csd)
    GetClientShadow(GTK_WINDOW(widget), &priv->client_shadow, &priv->bounds);
  // Set size constraints if needed.
  if (priv->needs_to_update_minmax_size) {
    if (priv->use_content_minmax_size)
      priv->delegate->SetContentSizeConstraints(priv->min_size, priv->max_size);
    else
      priv->delegate->SetSizeConstraints(priv->min_size, priv->max_size);
  }
  return FALSE;
}

// Window position/size has changed.
gboolean OnConfigure(GtkWidget* widget, GdkEventConfigure* event,
                     NUWindowPrivate* priv) {
  priv->bounds = RectF(event->x, event->y, event->width, event->height);
  if (priv->is_csd)
    priv->bounds.Inset(priv->client_shadow);
  return FALSE;
}

// Window state has changed.
gboolean OnWindowState(GtkWidget* widget, GdkEvent* event,
                       NUWindowPrivate* priv) {
  priv->window_state = event->window_state.new_window_state;
  return FALSE;
}

// Window has lost grab.
gboolean OnWindowGrabBroken(GtkWidget*, GdkEventGrabBroken*, Window* window) {
  GetPrivate(window)->captured_device = nullptr;
  window->on_capture_lost.Emit(window);
  return FALSE;
}

// When is-active property has changed.
void OnIsActiveChanged(GtkWidget* widget, GParamSpec*, Window* window) {
  if (window->IsActive())
    window->on_focus.Emit(window);
  else
    window->on_blur.Emit(window);
}

// Make window support alpha channel for the screen.
void OnScreenChanged(GtkWidget* widget, GdkScreen* old_screen, Window* window) {
  GdkScreen* screen = gtk_widget_get_screen(widget);
  GdkVisual* visual = gdk_screen_get_rgba_visual(screen);
  gtk_widget_set_visual(widget, visual);
}

// Set input shape for frameless transparent window.
gboolean OnDrawBackground(GtkWidget* widget, cairo_t* cr,
                          NUWindowPrivate* priv) {
  cairo_region_t* region = CreateRegionForNonAlphaArea(cr);
  gtk_widget_input_shape_combine_region(widget, region);
  cairo_region_destroy(region);
  // Only handle once.
  priv->is_draw_handler_set = false;
  priv->is_input_shape_set = true;
  g_signal_handler_disconnect(widget, priv->draw_handler_id);
  return FALSE;
}

// Get the height of menubar.
inline int GetMenuBarHeight(const Window* window) {
  int menu_bar_height = 0;
  if (window->GetMenuBar()) {
    gtk_widget_get_preferred_height(
        GTK_WIDGET(window->GetMenuBar()->GetNative()),
        &menu_bar_height, nullptr);
  }
  return menu_bar_height;
}

}  // namespace

void Window::PlatformInit(const Options& options) {
  window_ = GTK_WINDOW(gtk_window_new(
        options.no_activate ? GTK_WINDOW_POPUP : GTK_WINDOW_TOPLEVEL));

  NUWindowPrivate* priv = new NUWindowPrivate;
  priv->delegate = this;
  priv->is_popup = options.no_activate;
  g_object_set_data_full(G_OBJECT(window_), "private", priv,
                         Delete<NUWindowPrivate>);

  // Window is not focused by default.
  gtk_window_set_focus_on_map(window_, false);

  // Window events.
  g_signal_connect(window_, "delete-event", G_CALLBACK(OnDelete), this);
  g_signal_connect(window_, "map-event", G_CALLBACK(OnMap), priv);
  g_signal_connect(window_, "configure-event",
                   G_CALLBACK(OnConfigure), priv);
  g_signal_connect(window_, "window-state-event",
                   G_CALLBACK(OnWindowState), priv);
  g_signal_connect(window_, "grab-broken-event",
                   G_CALLBACK(OnWindowGrabBroken), this);
  g_signal_connect(window_, "notify::is-active",
                   G_CALLBACK(OnIsActiveChanged), this);

  // Lazy install event handlers.
  on_mouse_down.SetDelegate(this, kOnMouseClick);
  on_mouse_up.SetDelegate(this, kOnMouseClick);
  on_mouse_move.SetDelegate(this, kOnMouseMove);
  on_mouse_enter.SetDelegate(this, kOnMouseMove);
  on_mouse_leave.SetDelegate(this, kOnMouseMove);

  if (!options.frame && !priv->is_popup) {
    // Rely on client-side decoration to provide window features for frameless
    // window, like resizing and shadows.
    EnableCSD(window_);
    priv->is_csd = true;
  }

  if (options.transparent) {
    // Transparent background.
    gtk_widget_set_app_paintable(GTK_WIDGET(window_), true);
    // Set alpha channel in window.
    OnScreenChanged(GTK_WIDGET(window_), nullptr, this);
    g_signal_connect(window_, "screen-changed",
                     G_CALLBACK(OnScreenChanged), this);
  }

  // Must use a vbox to pack menubar.
  GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_container_add(GTK_CONTAINER(window_), vbox);
}

void Window::PlatformDestroy() {
  if (window_)
    gtk_widget_destroy(GTK_WIDGET(window_));
}

void Window::Close() {
  if (should_close && !should_close(this))
    return;

  NotifyWindowClosed();
  gtk_widget_destroy(GTK_WIDGET(window_));

  window_ = nullptr;
}

void Window::SetHasShadow(bool has) {
}

bool Window::HasShadow() const {
  return IsUsingCSD(window_);
}

void Window::PlatformSetContentView(View* view) {
  GtkContainer* vbox = GTK_CONTAINER(gtk_bin_get_child(GTK_BIN(window_)));
  if (content_view_)
    gtk_container_remove(vbox, content_view_->GetNative());
  gtk_container_add(vbox, view->GetNative());
  gtk_box_set_child_packing(GTK_BOX(vbox), view->GetNative(), TRUE, TRUE,
                            0, GTK_PACK_END);

  ForceSizeAllocation(window_, GTK_WIDGET(vbox));

  // For frameless transparent window, we need to set input shape to allow
  // click-through in transparent areas. For best performance we only set
  // input shape the first time when content view is drawn, GTK do redraws very
  // frequently and computing input shape is rather expensive.
  if (IsTransparent() && !HasFrame()) {
    NUWindowPrivate* priv = GetPrivate(this);
    if (!priv->is_draw_handler_set) {
      priv->is_draw_handler_set = true;
      priv->draw_handler_id = g_signal_connect_after(
          G_OBJECT(window_), "draw", G_CALLBACK(OnDrawBackground), priv);
    }
  }
}

void Window::SetContentSize(const SizeF& size) {
  // Menubar is part of client area in GTK.
  float height = size.height() + GetMenuBarHeight(this);
  ResizeWindow(window_, IsResizable(), size.width(), height);
  // Save the size request.
  NUWindowPrivate* priv = GetPrivate(this);
  priv->bounds.set_width(size.width());
  priv->bounds.set_height(height);
}

SizeF Window::GetContentSize() const {
  return GetContentBounds().size();
}

RectF Window::GetContentBounds() const {
  NUWindowPrivate* priv = GetPrivate(this);
  RectF cbounds = priv->bounds;
  // Popup window does not receive configure event, do not use cached bounds.
  if (priv->is_popup && gtk_widget_get_mapped(GTK_WIDGET(window_))) {
    GdkWindow* gdkwindow = gtk_widget_get_window(GTK_WIDGET(window_));
    if (gdkwindow) {
      int x, y, width, height;
      gdk_window_get_geometry(gdkwindow, &x, &y, &width, &height);
      cbounds = RectF(x, y, width, height);
    }
  }
  cbounds.Inset(0, GetMenuBarHeight(this), 0, 0);
  return cbounds;
}

void Window::SetBounds(const RectF& bounds) {
  NUWindowPrivate* priv = GetPrivate(this);
  // GTK does not count frame when resizing window.
  // FIXME(zcbenz): Setting bounds before window is mapped would result in
  // wrong window size, as the size of native frame is not available yet.
  RectF cbounds(bounds);
  cbounds.Inset(priv->native_frame);
  ResizeWindow(window_, IsResizable(), cbounds.width(), cbounds.height());
  gtk_window_move(window_, cbounds.x(), cbounds.y());
  // Save the size request.
  priv->bounds = cbounds;
}

RectF Window::GetBounds() const {
  return ContentBoundsToWindowBounds(GetContentBounds());
}

RectF Window::ContentBoundsToWindowBounds(const RectF& bounds) const {
  NUWindowPrivate* priv = GetPrivate(this);
  if (priv->is_popup)
    return bounds;
  RectF result = bounds;
  result.Inset(0, -GetMenuBarHeight(this), 0, 0);
  result.Inset(-priv->native_frame);
  return result;
}

RectF Window::WindowBoundsToContentBounds(const RectF& bounds) const {
  NUWindowPrivate* priv = GetPrivate(this);
  if (priv->is_popup)
    return bounds;
  RectF result = bounds;
  result.Inset(priv->native_frame);
  result.Inset(0, GetMenuBarHeight(this), 0, 0);
  return result;
}

void Window::SetSizeConstraints(const SizeF& min_size, const SizeF& max_size) {
  NUWindowPrivate* priv = GetPrivate(this);
  priv->use_content_minmax_size = false;
  priv->min_size = min_size;
  priv->max_size = max_size;

  // We can not defer the window frame if window is not mapped, so update
  // size constraints until window is mapped.
  if (!gtk_widget_get_mapped(GTK_WIDGET(window_))) {
    priv->needs_to_update_minmax_size = true;
    return;
  } else {
    priv->needs_to_update_minmax_size = false;
  }

  GdkGeometry hints = { 0 };
  int flags = 0;
  if (!min_size.IsEmpty()) {
    RectF bounds(min_size);
    bounds.Inset(priv->is_csd ? -priv->client_shadow : priv->native_frame);
    flags |= GDK_HINT_MIN_SIZE;
    hints.min_width = bounds.width();
    hints.min_height = bounds.height();
  }
  if (!max_size.IsEmpty()) {
    RectF bounds(max_size);
    bounds.Inset(priv->is_csd ? -priv->client_shadow : priv->native_frame);
    flags |= GDK_HINT_MAX_SIZE;
    hints.max_width = bounds.width();
    hints.max_height = bounds.height();
  }
  gtk_window_set_geometry_hints(window_, NULL, &hints, GdkWindowHints(flags));
}

std::tuple<SizeF, SizeF> Window::GetSizeConstraints() const {
  NUWindowPrivate* priv = GetPrivate(this);
  if (!priv->use_content_minmax_size)
    return std::make_tuple(priv->min_size, priv->max_size);
  return std::tuple<SizeF, SizeF>();
}

void Window::SetContentSizeConstraints(const SizeF& min_size,
                                       const SizeF& max_size) {
  NUWindowPrivate* priv = GetPrivate(this);
  priv->use_content_minmax_size = true;
  priv->min_size = min_size;
  priv->max_size = max_size;

  // We can not defer the window frame if window is not mapped, so update
  // size constraints until window is mapped.
  if (!gtk_widget_get_mapped(GTK_WIDGET(window_))) {
    priv->needs_to_update_minmax_size = true;
    return;
  } else {
    priv->needs_to_update_minmax_size = false;
  }

  GdkGeometry hints = { 0 };
  int flags = 0;
  if (!min_size.IsEmpty()) {
    RectF bounds(min_size);
    bounds.set_height(bounds.height() + GetMenuBarHeight(this));
    if (priv->is_csd)
      bounds.Inset(-priv->client_shadow);
    priv->min_size = bounds.size();
    flags |= GDK_HINT_MIN_SIZE;
    hints.min_width = bounds.width();
    hints.min_height = bounds.height();
  }
  if (!max_size.IsEmpty()) {
    RectF bounds(max_size);
    bounds.set_height(bounds.height() + GetMenuBarHeight(this));
    if (priv->is_csd)
      bounds.Inset(-priv->client_shadow);
    priv->max_size = bounds.size();
    flags |= GDK_HINT_MAX_SIZE;
    hints.max_width = bounds.width();
    hints.max_height = bounds.height();
  }
  gtk_window_set_geometry_hints(window_, NULL, &hints, GdkWindowHints(flags));
}

std::tuple<SizeF, SizeF> Window::GetContentSizeConstraints() const {
  NUWindowPrivate* priv = GetPrivate(this);
  if (priv->use_content_minmax_size)
    return std::make_tuple(priv->min_size, priv->max_size);
  return std::tuple<SizeF, SizeF>();
}

void Window::Activate() {
  if (!IsVisible())
    gtk_window_set_focus_on_map(window_, true);
  gtk_window_present(window_);
}

void Window::Deactivate() {
  gdk_window_lower(gtk_widget_get_window(GTK_WIDGET(window_)));
}

bool Window::IsActive() const {
  return gtk_window_is_active(window_);
}

void Window::SetVisible(bool visible) {
  gtk_widget_set_visible(GTK_WIDGET(window_), visible);
}

bool Window::IsVisible() const {
  return gtk_widget_get_visible(GTK_WIDGET(window_));
}

void Window::SetAlwaysOnTop(bool top) {
  gtk_window_set_keep_above(window_, top);
}

bool Window::IsAlwaysOnTop() const {
  return GetPrivate(this)->window_state & GDK_WINDOW_STATE_ABOVE;
}

void Window::SetFullscreen(bool fullscreen) {
  if (fullscreen)
    gtk_window_fullscreen(window_);
  else
    gtk_window_unfullscreen(window_);
}

bool Window::IsFullscreen() const {
  return GetPrivate(this)->window_state & GDK_WINDOW_STATE_FULLSCREEN;
}

void Window::Maximize() {
  gtk_window_maximize(window_);
}

void Window::Unmaximize() {
  gtk_window_unmaximize(window_);
}

bool Window::IsMaximized() const {
  return GetPrivate(this)->window_state & GDK_WINDOW_STATE_MAXIMIZED;
}

void Window::Minimize() {
  gtk_window_iconify(window_);
}

void Window::Restore() {
  gtk_window_deiconify(window_);
}

bool Window::IsMinimized() const {
  return GetPrivate(this)->window_state & GDK_WINDOW_STATE_ICONIFIED;
}

void Window::SetResizable(bool resizable) {
  if (resizable == IsResizable())
    return;

  // Current size of content view (gtk_window_get_size is not reliable when
  // window is not realized).
  GdkRectangle alloc;
  GtkWidget* vbox = gtk_bin_get_child(GTK_BIN(window_));
  gtk_widget_get_allocation(vbox, &alloc);

  // Prevent window from changing size when calling gtk_window_set_resizable.
  if (resizable) {
    // Clear the size requests for resizable window, otherwise window would have
    // a minimum size.
    gtk_widget_set_size_request(GTK_WIDGET(window_), -1, -1);
    gtk_widget_set_size_request(gtk_bin_get_child(GTK_BIN(window_)), -1, -1);
    // Window would be resized to default size for resizable window.
    gtk_window_set_default_size(window_, alloc.width, alloc.height);
  } else {
    // Set size requests for unresizable window, otherwise window would be
    // resize to whatever current size request is.
    ResizeWindow(window_, resizable, alloc.width, alloc.height);
  }

  gtk_window_set_resizable(window_, resizable);

  // For transparent window, using CSD means having extra shadow and border, so
  // we only use CSD when window is not resizable.
  if (!HasFrame() && IsTransparent()) {
    auto* priv = GetPrivate(this);
    if (priv->is_csd && !resizable) {
      DisableCSD(window_);
      priv->is_csd = false;
      if (gtk_widget_get_mapped(GTK_WIDGET(window_)))
        priv->client_shadow = InsetsF();
    } else if (!priv->is_csd && resizable) {
      EnableCSD(window_);
      priv->is_csd = true;
      if (gtk_widget_get_mapped(GTK_WIDGET(window_)))
        GetClientShadow(window_, &priv->client_shadow, &priv->bounds);
    }
  }
}

bool Window::IsResizable() const {
  return gtk_window_get_resizable(window_);
}

void Window::SetMaximizable(bool yes) {
  // In theory it is possible to set _NET_WM_ALLOWED_ACTIONS WM hint to
  // implement this, but as I have tried non major desktop environment supports
  // this hint, so on Linux this is simply impossible to implement.
}

bool Window::IsMaximizable() const {
  return IsResizable();
}

void Window::SetMinimizable(bool minimizable) {
  // See SetMaximizable for why this is not implemented.
}

bool Window::IsMinimizable() const {
  return true;
}

void Window::SetMovable(bool movable) {
  // See SetMaximizable for why this is not implemented.
}

bool Window::IsMovable() const {
  return true;
}

void Window::SetTitle(const std::string& title) {
  gtk_window_set_title(window_, title.c_str());
}

std::string Window::GetTitle() const {
  return gtk_window_get_title(window_);
}

void Window::SetBackgroundColor(Color color) {
  GdkRGBA gcolor = color.ToGdkRGBA();
  gtk_widget_override_background_color(GTK_WIDGET(window_),
                                       GTK_STATE_FLAG_NORMAL, &gcolor);
}

void Window::SetCapture() {
  if (HasCapture())
    ReleaseCapture();
  GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(window_));
  if (!window)
    return;
  GdkDevice* device = gtk_get_current_event_device();
  if (gdk_device_get_source(device) != GDK_SOURCE_MOUSE)
    device = gdk_device_get_associated_device(device);
  if (!device)
    return;
  const GdkEventMask mask = GdkEventMask(GDK_BUTTON_PRESS_MASK |
                                         GDK_BUTTON_RELEASE_MASK |
                                         GDK_POINTER_MOTION_HINT_MASK |
                                         GDK_POINTER_MOTION_MASK);
  gdk_device_grab(device, window, GDK_OWNERSHIP_WINDOW, TRUE, mask,
                  NULL, GDK_CURRENT_TIME);
  gtk_device_grab_add(GTK_WIDGET(window_), device, TRUE);
  GetPrivate(this)->captured_device = device;
}

void Window::ReleaseCapture() {
  GdkDevice* device = GetPrivate(this)->captured_device;
  if (!device)
    return;
  gdk_device_ungrab(device, GDK_CURRENT_TIME);
  gtk_device_grab_remove(GTK_WIDGET(window_), device);
}

bool Window::HasCapture() const {
  return GetPrivate(this)->captured_device;
}

void Window::SetSkipTaskbar(bool skip) {
  gtk_window_set_skip_taskbar_hint(window_, skip);
}

void Window::PlatformSetIcon(Image* icon) {
  gtk_window_set_icon(window_,
                      gdk_pixbuf_animation_get_static_image(icon->GetNative()));
}

void Window::PlatformSetMenuBar(MenuBar* menu_bar) {
  GtkContainer* vbox = GTK_CONTAINER(gtk_bin_get_child(GTK_BIN(window_)));
  if (menu_bar_)
    gtk_container_remove(vbox, GTK_WIDGET(menu_bar_->GetNative()));

  // Add menu to window's content vbox view.
  GtkWidget* menu = GTK_WIDGET(menu_bar->GetNative());
  gtk_container_add(vbox, menu);
  gtk_box_set_child_packing(GTK_BOX(vbox), menu, FALSE, FALSE, 0,
                            GTK_PACK_START);

  // Update the accelerator group.
  if (menu_bar_)
    gtk_window_remove_accel_group(window_,
                                  menu_bar_->accel_manager()->accel_group());
  gtk_window_add_accel_group(window_,
                             menu_bar->accel_manager()->accel_group());

  ForceSizeAllocation(window_, GTK_WIDGET(vbox));
}

void Window::PlatformAddChildWindow(Window* child) {
  gtk_window_set_transient_for(child->GetNative(), window_);
}

void Window::PlatformRemoveChildWindow(Window* child) {
  gtk_window_set_transient_for(child->GetNative(), nullptr);
}

}  // namespace nu
