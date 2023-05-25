// Copyright 2023 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/strings/pattern.h"
#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class LocaleTest : public testing::Test {
 protected:
  void SetUp() override {
  }
};

TEST_F(LocaleTest, GetCurrentIdentifier) {
  ASSERT_TRUE(base::MatchPattern(nu::Locale::GetCurrentIdentifier(), "*-*"));
}

TEST_F(LocaleTest, GetPreferredLanguages) {
#if defined(OS_LINUX) || defined(OS_MAC)
  // This test is weird on CI.
  if (::getenv("CI")) return;
#endif
  auto langs = nu::Locale::GetPreferredLanguages();
  ASSERT_GT(langs.size(), 0ul);
  ASSERT_TRUE(base::MatchPattern(langs[0], "*-*"));
}
