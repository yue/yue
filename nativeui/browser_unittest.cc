// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/json/json_writer.h"
#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

class BrowserTest : public testing::Test {
 protected:
  void SetUp() override {
    browser_ = new nu::Browser;
    nu::MessageLoop::PostTask([&]() {
      browser_->LoadURL("about:blank");
    });
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::Browser> browser_;
};

TEST_F(BrowserTest, OnFishiNavigation) {
  browser_->on_finish_navigation.Connect([](nu::Browser*) {
    nu::MessageLoop::Quit();
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, ExecuteJavaScript) {
  browser_->on_finish_navigation.Connect([](nu::Browser* browser) {
    browser->ExecuteJavaScript("location.href",
                               [](bool success, base::Value result) {
      nu::MessageLoop::Quit();
      ASSERT_EQ(success, true);
      ASSERT_TRUE(result.is_string());
      ASSERT_EQ(result.GetString(), "about:blank");
    });
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, ExecuteJavaScriptComplexResult) {
  browser_->on_finish_navigation.Connect([](nu::Browser* browser) {
    browser->ExecuteJavaScript("r = {a: true, b: {c: [], d: 'te' + 'st'}}; r",
                               [](bool success, base::Value result) {
      nu::MessageLoop::Quit();
      ASSERT_EQ(success, true);
      ASSERT_TRUE(result.is_dict());
      std::string json;
      ASSERT_TRUE(base::JSONWriter::Write(result, &json));
      ASSERT_EQ(json, "{\"a\":true,\"b\":{\"c\":[],\"d\":\"test\"}}");
    });
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, AddBinding) {
  bool bo = false;
  std::string st;
  base::Value va;
  std::function<void(bool, const std::string&, const base::Value&)> handler =
      [&](bool b, const std::string& s, const base::Value& v) {
        bo = b;
        st = s;
        va = v;
      };
  browser_->AddBinding("method", handler);
  browser_->on_finish_navigation.Connect([&](nu::Browser* browser) {
    browser->ExecuteJavaScript("window.method(true, 'string', {k: 'v'})",
                               [&](bool success, base::Value result) {
      nu::MessageLoop::Quit();
      ASSERT_EQ(success, true);
      ASSERT_EQ(bo, true);
      ASSERT_EQ(st, "string");
      ASSERT_TRUE(va.is_dict());
    });
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, SetBindingName) {
  browser_->SetBindingName("binding");
  browser_->AddRawBinding("method", [](base::Value) {});
  browser_->on_finish_navigation.Connect([](nu::Browser* browser) {
    browser->ExecuteJavaScript("window.method()",
                               [=](bool success, base::Value result) {
      EXPECT_EQ(success, false);
      browser->ExecuteJavaScript("window.binding.method()",
                                 [](bool success, base::Value result) {
        EXPECT_EQ(success, true);
        nu::MessageLoop::Quit();
      });
    });
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, MalicousCall) {
  browser_->AddRawBinding("method", [](base::Value) {});
  browser_->on_finish_navigation.Connect([](nu::Browser* browser) {
    browser->ExecuteJavaScript("window.external.postMessage('', 'method', '')",
                               [=](bool success, base::Value result) {
      EXPECT_EQ(success, false);
      nu::MessageLoop::Quit();
    });
  });
  nu::MessageLoop::Run();
}
