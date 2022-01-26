// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/date_picker.h"

#include <gtk/gtk.h>
#include <langinfo.h>

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/time/time.h"
#include "nativeui/container.h"
#include "nativeui/gtk/util/gtk_signal.h"
#include "nativeui/gtk/util/widget_util.h"
#include "nativeui/label.h"
#include "nativeui/popover.h"

namespace nu {

namespace {

class Calendar;
class Clock;

// Return localized time format for elements.
std::string GetFormatString(int elements) {
  // Shortcut for presets.
  if (elements == DatePicker::ELEMENT_YEAR_MONTH_DAY)
    return nl_langinfo(D_FMT);
  if (elements == DatePicker::ELEMENT_HOUR_MINUTE_SECOND)
    return nl_langinfo(T_FMT);
  if (elements == (DatePicker::ELEMENT_YEAR_MONTH_DAY |
                   DatePicker::ELEMENT_HOUR_MINUTE_SECOND))
    return nl_langinfo(D_T_FMT);
  // Manual concatenate of formats.
  std::string format;
  if ((elements & DatePicker::ELEMENT_YEAR_MONTH_DAY) ==
          DatePicker::ELEMENT_YEAR_MONTH_DAY) {
    format += nl_langinfo(D_FMT);
  } else if ((elements & DatePicker::ELEMENT_YEAR_MONTH) ==
                 DatePicker::ELEMENT_YEAR_MONTH) {
    // Return date in Month/Year or Year/Month
    std::string ymd = nl_langinfo(D_FMT);
    bool year_month = ymd == "%D" || ymd.find("%m") > ymd.find("%Y");
    format += year_month ? "%Y/%m" : "%m/%Y";
  }
  if (!format.empty())
    format += " ";
  if ((elements & DatePicker::ELEMENT_HOUR_MINUTE_SECOND) ==
          DatePicker::ELEMENT_HOUR_MINUTE_SECOND) {
    format += nl_langinfo(T_FMT);
  } else if ((elements & DatePicker::ELEMENT_HOUR_MINUTE) ==
                 DatePicker::ELEMENT_HOUR_MINUTE) {
    // Return time in 12 or 24 hours.
    std::string hms = nl_langinfo(T_FMT);
    bool twelve = hms == "%r" || hms.find("%I") != std::string::npos;
    format += twelve ? "%I:%M %p" : "%R";
  }
  return format;
}

struct NUDatePickerPrivate {
  DatePicker* delegate;

  int elements = 0;
  base::Time date = base::Time::Now();
  absl::optional<base::Time> min_date;
  absl::optional<base::Time> max_date;

  scoped_refptr<Popover> popover;
  Calendar* calendar;  // managed by popover
  Clock* clock;  // managed by popover

  static NUDatePickerPrivate* From(const DatePicker* picker) {
    return static_cast<NUDatePickerPrivate*>(
        g_object_get_data(G_OBJECT(picker->GetNative()), "datepicker-private"));
  }

  // Update the string representation of time.
  void UpdateDate();
  // Change part of the time.
  template<class T>
  void UpdatePartialDate(T* sender);
  // Create popover to provide a interface to select date and time.
  void CreatePopover();
  // Show/hide popover when the button is toggled.
  CHROMEGTK_CALLBACK_0(NUDatePickerPrivate, void, TogglePopover);
};

class Calendar : public View {
 public:
  explicit Calendar(NUDatePickerPrivate* priv) : priv_(priv) {
    TakeOverView(gtk_calendar_new());
    UpdateDefaultStyle();
    signal_day_changed_ =
        g_signal_connect(GetNative(), "day-selected",
                         G_CALLBACK(OnChangeThunk), this);
    signal_day_changed_double_click_ =
        g_signal_connect(GetNative(), "day-selected-double-click",
                         G_CALLBACK(OnDoubleClickThunk), this);
  }

  void SetDate(GDateTime* date) {
    auto* calendar = GTK_CALENDAR(GetNative());
    g_signal_handler_block(calendar, signal_day_changed_);
    gtk_calendar_select_month(calendar,
                              g_date_time_get_month(date) - 1,
                              g_date_time_get_year(date));
    gtk_calendar_select_day(calendar, g_date_time_get_day_of_month(date));
    g_signal_handler_unblock(calendar, signal_day_changed_);
  }

  void UpdatePartialDate(base::Time::Exploded* exploded) {
    // Get selected date from calendar
    guint year, month, day;
    gtk_calendar_get_date(GTK_CALENDAR(GetNative()), &year, &month, &day);
    // Set day/month/year without changing hour/minutes.
    exploded->year = year;
    exploded->month = month + 1;
    exploded->day_of_month = day;
  }

  // View:
  SizeF GetMinimumSize() const override {
    return GetPreferredSizeForWidget(GetNative());
  }

 private:
  ~Calendar() override = default;

  // User has selected a date in the calendar.
  CHROMEGTK_CALLBACK_0(Calendar, void, OnChange) {
    priv_->UpdatePartialDate(this);
  }

  // User double clicks the date in calendar.
  CHROMEGTK_CALLBACK_0(Calendar, void, OnDoubleClick) {
    DCHECK(priv_->popover);
    priv_->popover->Close();
  }

  NUDatePickerPrivate* priv_;
  gulong signal_day_changed_;
  gulong signal_day_changed_double_click_;
};

class ClockNumber : public View {
 public:
  ClockNumber(Clock* clock, int min, int max);

  void SetValue(double value) {
    g_signal_handler_block(GetNative(), signal_value_changed_);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(GetNative()), value);
    g_signal_handler_unblock(GetNative(), signal_value_changed_);
  }

  int GetValue() const {
    return static_cast<int>(
        gtk_spin_button_get_value(GTK_SPIN_BUTTON(GetNative())));
  }

  // View:
  SizeF GetMinimumSize() const override {
    return SizeF(50, 120);
  }

 private:
  ~ClockNumber() override = default;

  gulong signal_value_changed_;
};

class Clock : public Container {
 public:
  explicit Clock(NUDatePickerPrivate* priv)
      : priv_(priv),
        font_(Font::Default()->Derive(10, Font::Weight::Normal,
                                          Font::Style::Normal)),
        hour_(new ClockNumber(this, 0, 23)),
        minutes_(new ClockNumber(this, 0, 59)) {
    SetStyle("flex-direction", "row",
             "max-height", 120);
    AddChildView(hour_);
    AddChildView(CreateColon());
    AddChildView(minutes_);
    // Only show seconds when specified.
    int elements = priv->elements;
    if ((elements & DatePicker::ELEMENT_HOUR_MINUTE_SECOND) ==
            DatePicker::ELEMENT_HOUR_MINUTE_SECOND) {
      AddChildView(CreateColon());
      AddChildView(seconds_ = new ClockNumber(this, 0, 59));
    }
  }

  void SetTime(GDateTime* time) {
    hour_->SetValue(g_date_time_get_hour(time));
    minutes_->SetValue(g_date_time_get_minute(time));
    if (seconds_)
      seconds_->SetValue(g_date_time_get_second(time));
  }

  void UpdatePartialDate(base::Time::Exploded* exploded) {
    exploded->hour = hour_->GetValue();
    exploded->minute = minutes_->GetValue();
    if (seconds_)
      exploded->second = seconds_->GetValue();
  }

 private:
  friend class ClockNumber;

  ~Clock() override = default;

  scoped_refptr<Label> CreateColon() const {
    scoped_refptr<Label> colon = new Label(":");
    colon->SetFont(font_);
    colon->SetStyle("margin", 5,
                    "margin-top", -2);
    return colon;
  }

  // The number has been changed
  CHROMEGTK_CALLBACK_0(Clock, void, OnChange) {
    priv_->UpdatePartialDate(this);
  }

  NUDatePickerPrivate* priv_;
  scoped_refptr<Font> font_;
  ClockNumber* hour_;
  ClockNumber* minutes_;
  ClockNumber* seconds_ = nullptr;
};

void NUDatePickerPrivate::UpdateDate() {
  GDateTime* gdate = g_date_time_new_from_unix_local(date.ToTimeT());
  gchar* text = g_date_time_format(gdate, GetFormatString(elements).c_str());
  gtk_button_set_label(GTK_BUTTON(delegate->GetNative()), text);
  if (calendar)
    calendar->SetDate(gdate);
  if (clock)
    clock->SetTime(gdate);
  g_date_time_unref(gdate);
  g_free(text);
}

template<class T>
void NUDatePickerPrivate::UpdatePartialDate(T* sender) {
  // Get current date from picker.
  base::Time::Exploded exploded = {0};
  date.LocalExplode(&exploded);
  // Update partial date.
  sender->UpdatePartialDate(&exploded);
  base::Time new_date;
  if (!base::Time::FromLocalExploded(exploded, &new_date))
    LOG(WARNING) << "Time received from DatePicker is invalid";
  // Check range.
  if (max_date && new_date > *max_date)
    new_date = *max_date;
  if (min_date && new_date < *min_date)
    new_date = *min_date;
  delegate->SetDate(new_date);
}

void NUDatePickerPrivate::CreatePopover() {
  // Create elements.
  popover = new Popover();
  Container* content_view = new Container();
  content_view->SetStyle("flex-direction", "row");
  if (elements & DatePicker::ELEMENT_YEAR_MONTH) {
    calendar = new Calendar(this);
    content_view->AddChildView(calendar);
  }
  if (elements & DatePicker::ELEMENT_HOUR_MINUTE) {
    clock = new Clock(this);
    if (calendar)
      clock->SetStyle("margin-left", 10);
    content_view->AddChildView(clock);
  }
  // Calculate the size of popover.
  popover->SetContentView(content_view);
  popover->SetContentSize(content_view->GetPreferredSize());
  popover->on_close.Connect([this](Popover*) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(delegate->GetNative()),
                                 false);
  });
  UpdateDate();
}

void NUDatePickerPrivate::TogglePopover(GtkWidget* button) {
  bool active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
  if (active) {
    if (!popover)
      CreatePopover();
    popover->ShowRelativeTo(delegate);
  } else {
    DCHECK(popover);
    popover->Close();
  }
}

ClockNumber::ClockNumber(Clock* clock, int min, int max) {
  TakeOverView(gtk_spin_button_new_with_range(min, max, 1));
  SetFont(clock->font_);
  signal_value_changed_ =
      g_signal_connect(GetNative(), "value-changed",
                       G_CALLBACK(Clock::OnChangeThunk), clock);

  auto* spin = GTK_SPIN_BUTTON(GetNative());
  gtk_spin_button_set_digits(spin, 0);
  gtk_spin_button_set_wrap(spin, TRUE);
  gtk_spin_button_set_numeric(spin, TRUE);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(spin),
                                 GTK_ORIENTATION_VERTICAL);
  UpdateDefaultStyle();
}

}  // namespace

DatePicker::DatePicker(const Options& options) {
  GtkWidget* picker = gtk_toggle_button_new_with_label("DatePicker");
  auto* priv = new NUDatePickerPrivate{this, options.elements};
  g_object_set_data_full(G_OBJECT(picker), "datepicker-private", priv,
                         Delete<NUDatePickerPrivate>);
  g_signal_connect(picker, "toggled",
                   G_CALLBACK(NUDatePickerPrivate::TogglePopoverThunk), priv);
  TakeOverView(picker);
  priv->UpdateDate();
  UpdateDefaultStyle();
}

void DatePicker::SetDate(const base::Time& time) {
  auto* priv = NUDatePickerPrivate::From(this);
  priv->date = time;
  priv->UpdateDate();
}

base::Time DatePicker::GetDate() const {
  return NUDatePickerPrivate::From(this)->date;
}

void DatePicker::SetRange(const absl::optional<base::Time>& min,
                          const absl::optional<base::Time>& max) {
  auto* priv = NUDatePickerPrivate::From(this);
  priv->min_date = min;
  priv->max_date = max;
  priv->UpdateDate();
}

std::tuple<absl::optional<base::Time>,
           absl::optional<base::Time>> DatePicker::GetRange() const {
  auto* priv = NUDatePickerPrivate::From(this);
  return std::make_tuple(priv->min_date, priv->max_date);
}

bool DatePicker::HasStepper() const {
  return false;
}

SizeF DatePicker::GetMinimumSize() const {
  return GetPreferredSizeForWidget(GetNative());
}

}  // namespace nu
