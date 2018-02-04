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
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::Browser> browser_;
};

TEST_F(BrowserTest, LoadURL) {
  browser_->on_finish_navigation.Connect([](nu::Browser* browser,
                                            const std::string& url) {
    nu::MessageLoop::Quit();
    EXPECT_EQ(url, "about:blank");
    EXPECT_EQ(url, browser->GetURL());
  });
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadURL("about:blank");
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, LoadHTML) {
  browser_->on_finish_navigation.Connect([](nu::Browser*,
                                            const std::string& url) {
    nu::MessageLoop::Quit();
    EXPECT_EQ(url, "about:blank");
  });
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadHTML("<html></html>", "about:blank");
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, Navigation) {
  int state = 0;
  const std::string u = "about:blank";
  browser_->on_start_navigation.Connect([&](nu::Browser*,
                                            const std::string& url) {
    EXPECT_EQ(state, 0);
    EXPECT_EQ(url, u);
    state++;
  });
  browser_->on_commit_navigation.Connect([&](nu::Browser*,
                                             const std::string& url) {
    EXPECT_EQ(state, 1);
    EXPECT_EQ(url, u);
    state++;
  });
  browser_->on_finish_navigation.Connect([&](nu::Browser*,
                                             const std::string& url) {
    nu::MessageLoop::Quit();
    EXPECT_EQ(state, 2);
    EXPECT_EQ(url, u);
  });
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadURL(u);
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, LoadHTMLNavigation) {
  bool start_called = false;
  bool commit_called = false;
  bool finish_called = false;
  browser_->on_start_navigation.Connect([&](nu::Browser*,
                                            const std::string& url) {
    EXPECT_FALSE(start_called);
    start_called = true;
  });
  browser_->on_commit_navigation.Connect([&](nu::Browser*,
                                             const std::string& url) {
    EXPECT_FALSE(commit_called);
    commit_called = true;
  });
  browser_->on_finish_navigation.Connect([&](nu::Browser*,
                                             const std::string& url) {
    nu::MessageLoop::Quit();
    EXPECT_FALSE(finish_called);
    finish_called = true;
  });
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadHTML("<html></html>", "about:blank");
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, FailNavigation) {
  browser_->on_finish_navigation.Connect([](nu::Browser*, const std::string&) {
    ASSERT_TRUE(false) << "Should not emit FinishNavigation";
  });
  browser_->on_fail_navigation.Connect([](nu::Browser*,
                                          const std::string& url,
                                          int code) {
    nu::MessageLoop::Quit();
    EXPECT_NE(code, 0);
    EXPECT_EQ(url, "http://0.0.0.123/");
  });
  nu::MessageLoop::PostTask([&]() {
    // Load an invalid IP to fail quickly.
    browser_->LoadURL("http://0.0.0.123/");
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, LoadHTMLWithScript) {
  browser_->on_finish_navigation.Connect([](nu::Browser* browser,
                                            const std::string& url) {
    browser->ExecuteJavaScript("window.test",
                               [](bool success, base::Value result) {
      nu::MessageLoop::Quit();
      ASSERT_EQ(success, true);
      ASSERT_TRUE(result.is_string());
      ASSERT_EQ(result.GetString(), "true");
    });
  });
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadHTML("<html><script>window.test='true'</script></html>",
                       "about:blank");
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, LoadHTMLBaseURL) {
  browser_->on_finish_navigation.Connect([](nu::Browser* browser,
                                            const std::string& url) {
    browser->ExecuteJavaScript("var a = document.createElement('a');"
                               "a.href = 'relative';"
                               "a.href",
                               [](bool success, base::Value result) {
      nu::MessageLoop::Quit();
      ASSERT_EQ(success, true);
      ASSERT_TRUE(result.is_string());
      ASSERT_EQ(result.GetString(), "https://cheng.guru/relative");
    });
  });
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadHTML("<html><script>window.test='true'</script></html>",
                       "https://cheng.guru/");
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, Title) {
  std::string next_title = "t1";
  browser_->on_update_title.Connect([&](nu::Browser* browser,
                                        const std::string& title) {
    EXPECT_EQ(browser->GetTitle(), title);
    EXPECT_EQ(title, next_title);
    if (next_title == "t1") {
      next_title = "t2";
      browser->on_finish_navigation.Connect([](nu::Browser* browser,
                                               const std::string& url) {
        browser->ExecuteJavaScript("document.title = 't2'",
                                   [=](bool, base::Value) {
          EXPECT_EQ(browser->GetTitle(), "t2");
        });
      });
    } else {
      nu::MessageLoop::Quit();
    }
  });
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadHTML("<html><head><title>t1</title></head></html>",
                       "about:blank");
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, ExecuteJavaScript) {
  browser_->on_finish_navigation.Connect([](nu::Browser* browser,
                                            const std::string& url) {
    browser->ExecuteJavaScript("location.href",
                               [](bool success, base::Value result) {
      nu::MessageLoop::Quit();
      ASSERT_EQ(success, true);
      ASSERT_TRUE(result.is_string());
      ASSERT_EQ(result.GetString(), "about:blank");
    });
  });
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadURL("about:blank");
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, ExecuteJavaScriptComplexResult) {
  browser_->on_finish_navigation.Connect([](nu::Browser* browser,
                                            const std::string& url) {
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
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadURL("about:blank");
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, AddBinding) {
  bool bo = false;
  std::string st;
  base::Value va;
  std::function<void(bool, const std::string&, base::Value)> handler =
      [&](bool b, const std::string& s, base::Value v) {
        bo = b;
        st = s;
        va = std::move(v);
      };
  browser_->AddBinding("method", handler);
  browser_->on_finish_navigation.Connect([&](nu::Browser* browser,
                                             const std::string& url) {
    browser->ExecuteJavaScript("window.method(true, 'string', {k: 'v'})",
                               [&](bool success, base::Value result) {
      nu::MessageLoop::Quit();
      EXPECT_EQ(success, true);
      EXPECT_EQ(bo, true);
      EXPECT_EQ(st, "string");
      EXPECT_TRUE(va.is_dict());
    });
  });
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadHTML("<body><script></script></body>", "about:blank");
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, SetBindingName) {
  browser_->SetBindingName("binding");
  browser_->AddRawBinding("method", [](base::Value) {});
  browser_->on_finish_navigation.Connect([](nu::Browser* browser,
                                            const std::string& url) {
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
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadHTML("<body><script></script></body>", "about:blank");
  });
  nu::MessageLoop::Run();
}

TEST_F(BrowserTest, MalicousCall) {
  bool called = false;
  browser_->AddRawBinding("method", [&called](base::Value) { called = true; });
  browser_->on_finish_navigation.Connect([&called](nu::Browser* browser,
                                                   const std::string& url) {
    browser->ExecuteJavaScript(
#if defined(OS_WIN)
        "window.external.postMessage('1', 'method', '[]')",
#else
        "window.webkit.messageHandlers.yue.postMessage(['1', 'method', []])",
#endif
        [&called, browser](bool success, base::Value result) {
      EXPECT_EQ(called, false);
      browser->ExecuteJavaScript("window.method()",
                                 [&called](bool success, base::Value result) {
        nu::MessageLoop::Quit();
        EXPECT_EQ(called, false);
      });
    });
  });
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadHTML("<body><script></script></body>", "about:blank");
  });
  nu::MessageLoop::Run();
}
