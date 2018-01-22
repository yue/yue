// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/json/json_writer.h"
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
                               [](bool success, base::Value result) {
      EXPECT_EQ(success, true);
      EXPECT_TRUE(result.is_string());
      EXPECT_EQ(result.GetString(), "about:blank");
      nu::MessageLoop::Quit();
    });
  });
  nu::MessageLoop::PostTask([&]() {
    browser->LoadURL("about:blank");
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, ExecuteJavaScriptComplexResult) {
  scoped_refptr<nu::Browser> browser = new nu::Browser;
  browser->on_finish_navigation.Connect([](nu::Browser* browser) {
    browser->ExecuteJavaScript("r = {a: true, b: {c: [], d: 'te' + 'st'}}; r",
                               [](bool success, base::Value result) {
      EXPECT_EQ(success, true);
      EXPECT_TRUE(result.is_dict());
      std::string json;
      EXPECT_TRUE(base::JSONWriter::Write(result, &json));
      EXPECT_EQ(json, "{\"a\":true,\"b\":{\"c\":[],\"d\":\"test\"}}");
      nu::MessageLoop::Quit();
    });
  });
  nu::MessageLoop::PostTask([&]() {
    browser->LoadURL("about:blank");
  });
  nu::MessageLoop::Run();
}
