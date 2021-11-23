// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/screen.h"

#include <gtk/gtk.h>

#include <list>

#include "nativeui/window.h"

namespace nu {

namespace internal {

class ScreenObserverImpl : public ScreenObserver {
 public:
  explicit ScreenObserverImpl(Screen* screen) : screen_(screen) {
    GdkDisplay* display = gdk_display_get_default();
    if (!display)
      return;

    signal_monitor_added_ = g_signal_connect(
        display, "monitor-added", G_CALLBACK(OnMonitorAdded), this);
    signal_monitor_removed_ = g_signal_connect(
        display, "monitor-removed", G_CALLBACK(OnMonitorRemoved), this);

    int n_monitors = gdk_display_get_n_monitors(display);
    for (int i = 0; i < n_monitors; i++)
      AddMonitorSignals(gdk_display_get_monitor(display, i));
  }

  ~ScreenObserverImpl() override {
    GdkDisplay* display = gdk_display_get_default();
    if (display) {
      g_signal_handler_disconnect(display, signal_monitor_added_);
      g_signal_handler_disconnect(display, signal_monitor_removed_);
    }
    for (const auto& signal : monitor_signals_) {
      g_signal_handler_disconnect(signal.monitor, signal.geometry);
      g_signal_handler_disconnect(signal.monitor, signal.scale_factor);
      g_signal_handler_disconnect(signal.monitor, signal.work_area);
    }
  }

 private:
  struct MonitorSignal {
    GdkMonitor* monitor;
    ulong geometry;
    ulong scale_factor;
    ulong work_area;
  };

  void AddMonitorSignals(GdkMonitor* monitor) {
    MonitorSignal signal = {monitor};
    signal.geometry = g_signal_connect(
        monitor, "notify::geometry", G_CALLBACK(OnNotify), screen_);
    signal.scale_factor = g_signal_connect(
        monitor, "notify::scale-factor", G_CALLBACK(OnNotify), screen_);
    signal.work_area = g_signal_connect(
        monitor, "notify::workarea", G_CALLBACK(OnNotify), screen_);
    monitor_signals_.push_back(signal);
  }

  static void OnMonitorAdded(GdkDisplay*, GdkMonitor* monitor,
                             ScreenObserverImpl* self) {
    self->AddMonitorSignals(monitor);
    self->screen_->NotifyDisplaysChange();
  }

  static void OnMonitorRemoved(GdkDisplay*, GdkMonitor* monitor,
                               ScreenObserverImpl* self) {
    self->monitor_signals_.remove_if([monitor](const MonitorSignal& signal) {
      return signal.monitor == monitor;
    });
    self->screen_->NotifyDisplaysChange();
  }

  static void OnNotify(GdkDisplay*, GParamSpec*, Screen* screen) {
    screen->NotifyDisplaysChange();
  }

  ulong signal_monitor_added_ = 0;
  ulong signal_monitor_removed_ = 0;
  std::list<MonitorSignal> monitor_signals_;

  Screen* screen_;
};

// static
ScreenObserver* ScreenObserver::Create(Screen* screen) {
  return new ScreenObserverImpl(screen);
}

}  // namespace internal

// static
float Screen::GetDefaultScaleFactor() {
  GdkMonitor* monitor = GetNativePrimaryDisplay();
  if (!monitor)
    return 1.f;
  return gdk_monitor_get_scale_factor(monitor);
}

Display Screen::GetDisplayNearestWindow(Window* window) {
  GdkWindow* gdkwindow = gtk_widget_get_window(GTK_WIDGET(window->GetNative()));
  if (!GDK_IS_WINDOW(gdkwindow))
    return GetPrimaryDisplay();
  GdkDisplay* display = gdk_display_get_default();
  GdkMonitor* monitor = gdk_display_get_monitor_at_window(display, gdkwindow);
  if (!monitor)
    return GetPrimaryDisplay();
  return FindDisplay(monitor);
}

Display Screen::GetDisplayNearestPoint(const PointF& point) {
  GdkDisplay* display = gdk_display_get_default();
  GdkMonitor* monitor = gdk_display_get_monitor_at_point(display,
                                                         point.x(), point.y());
  if (!monitor)
    return GetPrimaryDisplay();
  return FindDisplay(monitor);
}

PointF Screen::GetCursorScreenPoint() {
  GdkDisplay* display = gdk_display_get_default();
  GdkSeat* seat = gdk_display_get_default_seat(display);
  GdkDevice* mouse_device = gdk_seat_get_pointer(seat);
  GdkWindow* window = gdk_get_default_root_window();
  gint x = 0, y = 0;
  gdk_window_get_device_position(window, mouse_device, &x, &y, nullptr);
  return PointF(x, y);
}

// static
uint32_t Screen::DisplayIdFromNative(GdkMonitor* monitor) {
  // Use pointer to construct ID since GTK does not change the monitor's pointer
  // for existing monitor.
  using Decouple = union {
    struct {
      uint32_t a;
      uint32_t b;
    } i32;
    uint64_t i64;
  };
  Decouple decouple;
  decouple.i64 = reinterpret_cast<uint64_t>(monitor);
  return decouple.i32.a + decouple.i32.b;
}

// static
NativeDisplay Screen::GetNativePrimaryDisplay() {
  return gdk_display_get_primary_monitor(gdk_display_get_default());
}

// static
Display Screen::CreateDisplayFromNative(GdkMonitor* monitor) {
  Display display;
  display.id = DisplayIdFromNative(monitor);
  display.scale_factor = gdk_monitor_get_scale_factor(monitor);

  GdkRectangle frame;
  gdk_monitor_get_geometry(monitor, &frame);
  display.bounds = RectF(Rect(frame));
  GdkRectangle work_area;
  gdk_monitor_get_workarea(monitor, &work_area);
  display.work_area = RectF(Rect(work_area));
  return display;
}

// static
Screen::DisplayList Screen::CreateAllDisplays() {
  GdkDisplay* display = gdk_display_get_default();
  int n_monitors = gdk_display_get_n_monitors(display);

  DisplayList displays;
  displays.reserve(n_monitors);
  for (int i = 0; i < n_monitors; i++) {
    GdkMonitor* monitor = gdk_display_get_monitor(display, i);
    displays.emplace_back(CreateDisplayFromNative(monitor));
  }

  if (displays.empty())
    return {Display()};
  return displays;
}

}  // namespace nu
