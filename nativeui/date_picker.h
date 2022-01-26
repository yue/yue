// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_DATE_PICKER_H_
#define NATIVEUI_DATE_PICKER_H_

#include <tuple>

#include "nativeui/view.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace base {
class Time;
}

namespace nu {

class NATIVEUI_EXPORT DatePicker : public View {
 public:
  // Numbers should match NSDatePickerElementFlags.
  enum Element {
    ELEMENT_YEAR_MONTH = 0x00C0,
    ELEMENT_YEAR_MONTH_DAY = 0x00E0,
    ELEMENT_HOUR_MINUTE = 0x000C,
    ELEMENT_HOUR_MINUTE_SECOND = 0x000E,
  };

  struct Options {
    int elements = ELEMENT_YEAR_MONTH_DAY;
    bool has_stepper = true;
  };

  explicit DatePicker(const Options& options);

  // View class name.
  static const char kClassName[];

  void SetDate(const base::Time& time);
  base::Time GetDate() const;
  void SetRange(const absl::optional<base::Time>& min,
                const absl::optional<base::Time>& max);
  std::tuple<absl::optional<base::Time>,
             absl::optional<base::Time>> GetRange() const;
  bool HasStepper() const;

  // View:
  const char* GetClassName() const override;
  SizeF GetMinimumSize() const override;

  // Events.
  Signal<void(DatePicker*)> on_date_change;

 protected:
  ~DatePicker() override;
};

}  // namespace nu

#endif  // NATIVEUI_DATE_PICKER_H_
