// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/time/time.h"
#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class DatePickerTest : public testing::Test {
 protected:
  nu::Lifetime lifetime_;
  nu::State state_;
};

TEST_F(DatePickerTest, Default) {
  scoped_refptr<nu::DatePicker> picker(new nu::DatePicker({}));
#if defined(OS_MAC) || defined(OS_WIN)
  ASSERT_TRUE(picker->HasStepper());
#endif
  auto range = picker->GetRange();
  EXPECT_FALSE(std::get<0>(range));
  EXPECT_FALSE(std::get<1>(range));
  // Initialized to current time.
  base::Time now = base::Time::Now();
  base::Time date = picker->GetDate();
  ASSERT_LT((now - date).InSeconds(), 1);
}

TEST_F(DatePickerTest, SetDate) {
  scoped_refptr<nu::DatePicker> picker(new nu::DatePicker({}));
  base::Time time;
  ASSERT_TRUE(base::Time::FromString("7 Oct 1990", &time));
  picker->SetDate(time);
  ASSERT_EQ(picker->GetDate(), time);
}

TEST_F(DatePickerTest, SetRange) {
  base::Time time1;
  ASSERT_TRUE(base::Time::FromString("15 Mar 1988", &time1));
  base::Time time2;
  ASSERT_TRUE(base::Time::FromString("7 Oct 1990", &time2));

  scoped_refptr<nu::DatePicker> picker(new nu::DatePicker({}));
  picker->SetRange(time1, absl::nullopt);
  auto range = picker->GetRange();
  EXPECT_TRUE(std::get<0>(range));
  EXPECT_FALSE(std::get<1>(range));
  EXPECT_EQ(time1, *std::get<0>(range));

  picker->SetRange(time1, time2);
  range = picker->GetRange();
  EXPECT_TRUE(std::get<0>(range));
  EXPECT_TRUE(std::get<1>(range));
  EXPECT_EQ(time1, *std::get<0>(range));
  EXPECT_EQ(time2, *std::get<1>(range));

  picker->SetRange(absl::nullopt, absl::nullopt);
  range = picker->GetRange();
  EXPECT_FALSE(std::get<0>(range));
  EXPECT_FALSE(std::get<1>(range));
}
