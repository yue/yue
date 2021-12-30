// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/base_paths.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/json/json_writer.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "nativeui/nativeui.h"
#include "testing/gtest/include/gtest/gtest.h"

enum TestOptions {
  DEFAULT,
#if defined(OS_WIN) && defined(WEBVIEW2_SUPPORT)
  WEBVIEW2,
  WEBVIEW2_IE,
#endif
};

class BrowserTest : public testing::TestWithParam<TestOptions> {
 protected:
  void SetUp() override {
    nu::Browser::Options options;
#if defined(OS_WIN) && defined(WEBVIEW2_SUPPORT)
    if (GetParam() == WEBVIEW2 || GetParam() == WEBVIEW2_IE)
      options.webview2_support = true;
    if (GetParam() == WEBVIEW2_IE)
      options.webview2_force_ie = true;
#endif
    browser_ = new nu::Browser(options);
  }

  nu::Lifetime lifetime_;
  nu::State state_;
  scoped_refptr<nu::Browser> browser_;
};

#if defined(OS_WIN) && defined(WEBVIEW2_SUPPORT)
TEST_P(BrowserTest, WebView2) {
  TestOptions param = GetParam();
  browser_->on_finish_navigation.Connect([=](nu::Browser* browser,
                                             const std::string& url) {
    nu::MessageLoop::Quit();
    if (param == DEFAULT)
      EXPECT_EQ(browser->IsWebView2(), false);
    else if (param == WEBVIEW2)
      EXPECT_EQ(browser->IsWebView2(), true);
    else if (param == WEBVIEW2_IE)
      EXPECT_EQ(browser->IsWebView2(), false);
  });
  browser_->LoadURL("about:blank");
  nu::MessageLoop::Run();
}
#endif

TEST_P(BrowserTest, DestroyAndCreate) {
  browser_ = nullptr;
  browser_ = new nu::Browser(nu::Browser::Options());
}

TEST_P(BrowserTest, LoadURL) {
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

TEST_P(BrowserTest, LoadHTML) {
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

TEST_P(BrowserTest, Navigation) {
  int state = 0;
  const std::string u = "http://example.com/";
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

TEST_P(BrowserTest, LoadHTMLNavigation) {
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

TEST_P(BrowserTest, FailNavigation) {
  browser_->on_finish_navigation.Connect([](nu::Browser*, const std::string&) {
    ASSERT_TRUE(false) << "Should not emit FinishNavigation";
  });
  browser_->on_fail_navigation.Connect([](nu::Browser*,
                                          const std::string& url,
                                          int code) {
    nu::MessageLoop::Quit();
    EXPECT_NE(code, 0);
    EXPECT_EQ(url, "http://host.invalid/");
  });
  nu::MessageLoop::PostTask([&]() {
    // Load an invalid IP to fail quickly.
    browser_->LoadURL("http://host.invalid/");
  });
  nu::MessageLoop::Run();
}

TEST_P(BrowserTest, LoadHTMLWithScript) {
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

TEST_P(BrowserTest, LoadHTMLBaseURL) {
#if defined(OS_WIN) && defined(WEBVIEW2_SUPPORT)
  if (browser_->IsWebView2())
    return;
#endif
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

TEST_P(BrowserTest, Title) {
  bool loaded = false;
  std::string next_title = "t1";
  browser_->on_finish_navigation.Connect([&loaded](nu::Browser*,
                                                   const std::string&) {
    loaded = true;
  });
  browser_->on_update_title.Connect([&](nu::Browser* browser,
                                        const std::string& title) {
    if (title.empty() || title == "about:blank")
      return;
    EXPECT_EQ(browser->GetTitle(), title);
    EXPECT_EQ(title, next_title);
    if (next_title == "t1") {
      next_title = "t2";
      if (loaded) {
        browser->ExecuteJavaScript("document.title = 't2'", nullptr);
      } else {
        browser->on_finish_navigation.Connect([](nu::Browser* browser,
                                                 const std::string& url) {
          browser->ExecuteJavaScript("document.title = 't2'", nullptr);
        });
      }
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

TEST_P(BrowserTest, UserAgent) {
#if defined(OS_WIN) && defined(WEBVIEW2_SUPPORT)
  if (browser_->IsWebView2())
    return;
#endif
  std::string user_agent = "MyBrowser v1.0";
  browser_->SetUserAgent(user_agent);
  browser_->on_finish_navigation.Connect([&](nu::Browser* browser,
                                             const std::string& url) {
    browser->ExecuteJavaScript("navigator.userAgent",
                               [&](bool success, base::Value result) {
      nu::MessageLoop::Quit();
      ASSERT_TRUE(result.is_string());
      ASSERT_EQ(result.GetString(), user_agent);
    });
  });
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadURL("about:blank");
  });
  nu::MessageLoop::Run();
}

TEST_P(BrowserTest, ExecuteJavaScript) {
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

TEST_P(BrowserTest, ExecuteJavaScriptComplexResult) {
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

TEST_P(BrowserTest, AddBinding) {
  std::function<void(nu::Browser*, bool, const std::string&, base::Value)>
      handler = [](nu::Browser*, bool b, const std::string& s, base::Value v) {
    nu::MessageLoop::Quit();
    EXPECT_EQ(b, true);
    EXPECT_EQ(s, "string");
    EXPECT_TRUE(v.is_dict());
  };
  browser_->AddBinding("method", handler);
  browser_->on_finish_navigation.Connect([&](nu::Browser* browser,
                                             const std::string& url) {
    browser->ExecuteJavaScript("window.method(true, 'string', {k: 'v'})",
                               nullptr);
  });
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadHTML("<body><script></script></body>", "about:blank");
  });
  nu::MessageLoop::Run();
}

void DummyFunction(const std::string&) {
}

TEST_P(BrowserTest, AddBindingFunctionPointer) {
  browser_->AddBinding("method", &DummyFunction);
}

TEST_P(BrowserTest, AddBindingCapturelessLabmda) {
  browser_->AddBinding("method", []() {});
}

TEST_P(BrowserTest, SetBindingName) {
  browser_->SetBindingName("binding");
  browser_->AddRawBinding("method", [](nu::Browser*, base::Value) {});
  browser_->on_finish_navigation.Connect([](nu::Browser* browser,
                                            const std::string& url) {
    browser->ExecuteJavaScript("window.method()",
                               [=](bool success, base::Value result) {
#if defined(OS_WIN) && defined(WEBVIEW2_SUPPORT)
      if (!browser->IsWebView2())
#endif
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

TEST_P(BrowserTest, MalicousCall) {
  bool called = false;
  browser_->AddRawBinding("method", [&called](nu::Browser*, base::Value) {
    called = true;
  });
  browser_->on_finish_navigation.Connect([&called](nu::Browser* browser,
                                                   const std::string& url) {
      std::string kPostMessage =
#if defined(OS_WIN)
#if defined(WEBVIEW2_SUPPORT)
        browser->IsWebView2() ? "window.chrome.webview.postMessage" :
#endif
                                "window.external.postMessage";
#else
        "window.webkit.messageHandlers.yue.postMessage";
#endif
    browser->ExecuteJavaScript(
        kPostMessage + "('[\"1\", \"method\", []]')",
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

TEST_P(BrowserTest, StringProtocol) {
#if defined(OS_WIN) && defined(WEBVIEW2_SUPPORT)
  if (browser_->IsWebView2())
    return;
#endif
  browser_->on_finish_navigation.Connect([](nu::Browser* browser,
                                            const std::string& url) {
    EXPECT_EQ(url, "str://host/path");
    browser->ExecuteJavaScript("document.body.textContent",
                               [=](bool success, base::Value result) {
      nu::Browser::UnregisterProtocol("str");
      nu::MessageLoop::Quit();
      ASSERT_TRUE(result.is_string());
      EXPECT_EQ(result.GetString(), "str://host/path");
    });
  });
  nu::Browser::RegisterProtocol("str", [](const std::string& url) {
    EXPECT_EQ(url, "str://host/path");
    return new nu::ProtocolStringJob("text/html",
                                     "<html><body>" + url + "</body></html>");
  });
  nu::MessageLoop::PostTask([&]() {
    browser_->LoadURL("str://host/path");
  });
  nu::MessageLoop::Run();
}

TEST_P(BrowserTest, FileProtocol) {
#if defined(OS_WIN) && defined(WEBVIEW2_SUPPORT)
  if (browser_->IsWebView2())
    return;
#endif
  // Write html to file.
  base::ScopedTempDir dir;
  ASSERT_TRUE(dir.CreateUniqueTempDir());
  base::FilePath file = dir.GetPath().Append(FILE_PATH_LITERAL("index.html"));
  std::string content = "<html><body>file</body></html>";
  base::WriteFile(file, content.c_str(), static_cast<int>(content.size()));
  // Register handler to read file.
  nu::Browser::RegisterProtocol("wenjian", [](const std::string& url) {
    std::string path = url.substr(15);
    return new nu::ProtocolFileJob(base::FilePath::FromUTF8Unsafe(path));
  });
  // Start test.
  browser_->on_finish_navigation.Connect([](nu::Browser* browser,
                                            const std::string& url) {
    browser->ExecuteJavaScript("document.body.textContent",
                               [](bool success, base::Value result) {
      nu::Browser::UnregisterProtocol("str");
      nu::MessageLoop::Quit();
      ASSERT_TRUE(result.is_string());
      EXPECT_EQ(result.GetString(), "file");
    });
  });
  nu::MessageLoop::PostTask([=]() {
    browser_->LoadURL("wenjian://read/" + file.AsUTF8Unsafe());
  });
  nu::MessageLoop::Run();
}

TEST_P(BrowserTest, LargeFileProtocol) {
#if defined(OS_WIN) && defined(WEBVIEW2_SUPPORT)
  if (browser_->IsWebView2())
    return;
#endif
  // Serve the pug.js, which should be large enough.
  base::FilePath exe_path;
  base::PathService::Get(base::FILE_EXE, &exe_path);
  base::FilePath file =
      exe_path.DirName().DirName().DirName()
              .Append(FILE_PATH_LITERAL("third_party"))
              .Append(FILE_PATH_LITERAL("bundled_node_modules"))
              .Append(FILE_PATH_LITERAL("pug.js"));
  std::string content;
  ASSERT_TRUE(base::ReadFileToString(file, &content));
  nu::Browser::RegisterProtocol("large", [&](const std::string& url) {
    return new nu::ProtocolStringJob(
        "text/html",
        "<html><body><script id='s'>" + content + "</script>"
        "<div id='after'>text</div></body></html>");
  });
  browser_->on_finish_navigation.Connect([](nu::Browser* browser,
                                            const std::string& url) {
    std::string command =
        "document.getElementById('after').textContent.length === 4";
    browser->ExecuteJavaScript(command, [](bool success, base::Value result) {
      nu::Browser::UnregisterProtocol("large");
      nu::MessageLoop::Quit();
      ASSERT_TRUE(result.is_bool());
      EXPECT_TRUE(result.GetBool());
    });
  });
  nu::MessageLoop::PostTask([=]() {
    browser_->LoadURL("large://file");
  });
  nu::MessageLoop::Run();
}

using ::testing::Values;

#if defined(OS_WIN)
INSTANTIATE_TEST_SUITE_P(IE, BrowserTest, Values(DEFAULT));
#if defined(WEBVIEW2_SUPPORT)
INSTANTIATE_TEST_SUITE_P(WebView2, BrowserTest,
                         Values(WEBVIEW2));
INSTANTIATE_TEST_SUITE_P(WebView2FallbackIE, BrowserTest,
                         Values(WEBVIEW2_IE));
#endif  // defined(WEBVIEW2_SUPPORT)
#else
INSTANTIATE_TEST_SUITE_P(WebKit, BrowserTest, Values(DEFAULT));
#endif
