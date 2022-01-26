// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/date_picker.h"

#include <commctrl.h>

#include "base/time/time.h"
#include "nativeui/win/subwin_view.h"

namespace nu {

namespace {

class DatePickerImpl : public SubwinView {
 public:
  DatePickerImpl(int style, DatePicker* delegate)
      : SubwinView(delegate, DATETIMEPICK_CLASS,
                   style | WS_CHILD | WS_VISIBLE) {
    set_focusable(true);
  }

  // SubwinView:
  LRESULT OnNotify(int, LPNMHDR pnmh) override {
    DatePicker* picker = static_cast<DatePicker*>(delegate());
    if (pnmh->code == DTN_DATETIMECHANGE)
      picker->on_date_change.Emit(picker);
    return 0;
  }
};

SYSTEMTIME BaseTimeToSystemTime(const base::Time& time) {
  base::Time::Exploded exploded = {0};
  time.LocalExplode(&exploded);
  SYSTEMTIME systemtime = {
    exploded.year,
    exploded.month,
    exploded.day_of_week,
    exploded.day_of_month,
    exploded.hour,
    exploded.minute,
    exploded.second,
    exploded.millisecond,
  };
  return systemtime;
}

base::Time SystemTimeToBaseTime(const SYSTEMTIME& systemtime) {
  base::Time::Exploded exploded = {
    systemtime.wYear,
    systemtime.wMonth,
    systemtime.wDayOfWeek,
    systemtime.wDay,
    systemtime.wHour,
    systemtime.wMinute,
    systemtime.wSecond,
    systemtime.wMilliseconds,
  };
  base::Time time;
  if (!base::Time::FromLocalExploded(exploded, &time))
    LOG(WARNING) << "Time received from DatePicker is invalid";
  return time;
}

std::wstring GetLocaleInfoString(LCTYPE type) {
  int buffer_size_with_nul = ::GetLocaleInfoEx(
      LOCALE_NAME_USER_DEFAULT, type, nullptr, 0);
  if (buffer_size_with_nul <= 1)
    return std::wstring();
  std::wstring buffer(buffer_size_with_nul - 1, L'\0');
  ::GetLocaleInfoEx(
      LOCALE_NAME_USER_DEFAULT, type, &buffer.front(), buffer_size_with_nul);
  return buffer;
}

std::wstring GetFormatString(int elements) {
  std::wstring format;
  if ((elements & DatePicker::ELEMENT_YEAR_MONTH_DAY) ==
          DatePicker::ELEMENT_YEAR_MONTH_DAY)
    format += GetLocaleInfoString(LOCALE_SSHORTDATE);
  else if ((elements & DatePicker::ELEMENT_YEAR_MONTH) ==
               DatePicker::ELEMENT_YEAR_MONTH)
    format += GetLocaleInfoString(LOCALE_SYEARMONTH);
  if (!format.empty())
    format += L" ";
  if ((elements & DatePicker::ELEMENT_HOUR_MINUTE_SECOND) ==
          DatePicker::ELEMENT_HOUR_MINUTE_SECOND)
    format += GetLocaleInfoString(LOCALE_STIMEFORMAT);
  else if ((elements & DatePicker::ELEMENT_HOUR_MINUTE) ==
               DatePicker::ELEMENT_HOUR_MINUTE)
    format += GetLocaleInfoString(LOCALE_SSHORTTIME);
  return format;
}

}  // namespace

DatePicker::DatePicker(const Options& options) {
  int style = 0;
  bool custom_format = false;
  if (options.has_stepper)
    style |= DTS_UPDOWN;
  if (options.elements == ELEMENT_YEAR_MONTH_DAY)
    style |= DTS_SHORTDATECENTURYFORMAT;
  else if (options.elements == ELEMENT_HOUR_MINUTE_SECOND)
    style |= DTS_TIMEFORMAT;
  else
    custom_format = true;

  auto* picker = new DatePickerImpl(style, this);
  // Use custom format when not using stock representations.
  if (custom_format) {
    std::wstring format = GetFormatString(options.elements);
    DateTime_SetFormat(picker->hwnd(), format.c_str());
  }
  // The default range is an invalid date, clear with explicit no range.
  DateTime_SetRange(picker->hwnd(), 0, nullptr);

  TakeOverView(picker);
  UpdateDefaultStyle();
}

void DatePicker::SetDate(const base::Time& time) {
  SYSTEMTIME systemtime = BaseTimeToSystemTime(time);
  DateTime_SetSystemtime(static_cast<DatePickerImpl*>(GetNative())->hwnd(),
                         GDT_VALID, &systemtime);
}

base::Time DatePicker::GetDate() const {
  SYSTEMTIME systemtime = {0};
  if (DateTime_GetSystemtime(static_cast<DatePickerImpl*>(GetNative())->hwnd(),
                             &systemtime) != GDT_VALID)
    return base::Time();
  return SystemTimeToBaseTime(systemtime);
}

void DatePicker::SetRange(const absl::optional<base::Time>& min,
                          const absl::optional<base::Time>& max) {
  WPARAM flags = 0;
  SYSTEMTIME systemtimes[2] = {0};
  if (min) {
    flags |= GDTR_MIN;
    systemtimes[0] = BaseTimeToSystemTime(*min);
  }
  if (max) {
    flags |= GDTR_MAX;
    systemtimes[1] = BaseTimeToSystemTime(*max);
  }
  DateTime_SetRange(static_cast<DatePickerImpl*>(GetNative())->hwnd(),
                    flags, &systemtimes);
}

std::tuple<absl::optional<base::Time>,
           absl::optional<base::Time>> DatePicker::GetRange() const {
  SYSTEMTIME systemtimes[2] = {0};
  DWORD flags = DateTime_GetRange(
      static_cast<DatePickerImpl*>(GetNative())->hwnd(), &systemtimes);
  return std::make_tuple(
      flags & GDTR_MIN ? SystemTimeToBaseTime(systemtimes[0])
                       : absl::optional<base::Time>(),
      flags & GDTR_MAX ? SystemTimeToBaseTime(systemtimes[1])
                       : absl::optional<base::Time>());
}

bool DatePicker::HasStepper() const {
  auto* picker = static_cast<DatePickerImpl*>(GetNative());
  return picker->HasWindowStyle(DTS_UPDOWN);
}

SizeF DatePicker::GetMinimumSize() const {
  return SizeF(50, 20);
}

}  // namespace nu
