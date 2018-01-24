// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class BrowserTest : public testing::Test {
 protected:
  void SetUp() override {
  }

  nu::Lifetime lifetime_;
  nu::State state_;
};

TEST_F(BrowserTest, LoadURL) {
  scoped_refptr<nu::Browser> browser = new nu::Browser;
  browser->on_finish_navigation.Connect([](nu::Browser*) {
    nu::MessageLoop::Quit();
  });
  nu::MessageLoop::PostTask([&]() {
    browser->LoadURL("about:blank");
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, ExecuteJavaScript) {
  scoped_refptr<nu::Browser> browser = new nu::Browser;
  browser->on_finish_navigation.Connect([](nu::Browser* browser) {
    browser->ExecuteJavaScript("location.href",
                               [](bool success, const std::string& result) {
      EXPECT_EQ(success, true);
      EXPECT_EQ(result, "\"about:blank\"");
    });
    nu::MessageLoop::Quit();
  });
  nu::MessageLoop::PostTask([&]() {
    browser->LoadURL("about:blank");
  });
  nu::MessageLoop::Run();
}
