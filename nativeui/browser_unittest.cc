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

class BrowserTest : public testing::Test {
 protected:
  void SetUp() override {
    browser_ = new nu::Browser(nu::Browser::Options());
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
    if (title.empty() || title == "about:blank")
      return;
    EXPECT_EQ(browser->GetTitle(), title);
    EXPECT_EQ(title, next_title);
    if (next_title == "t1") {
      next_title = "t2";
      browser->on_finish_navigation.Connect([](nu::Browser* browser,
                                               const std::string& url) {
        browser->ExecuteJavaScript("document.title = 't2'", nullptr);
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

TEST_F(BrowserTest, UserAgent) {
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
  std::function<void(nu::Browser*, bool, const std::string&, base::Value)>
      handler = [&](nu::Browser*, bool b, const std::string& s, base::Value v) {
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

void DummyFunction(const std::string&) {
}

TEST_F(BrowserTest, AddBindingFunctionPointer) {
  browser_->AddBinding("method", &DummyFunction);
}

TEST_F(BrowserTest, AddBindingCapturelessLabmda) {
  browser_->AddBinding("method", []() {});
}

TEST_F(BrowserTest, SetBindingName) {
  browser_->SetBindingName("binding");
  browser_->AddRawBinding("method", [](nu::Browser*, base::Value) {});
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
  browser_->AddRawBinding("method", [&called](nu::Browser*, base::Value) {
    called = true;
  });
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

TEST_F(BrowserTest, StringProtocol) {
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

TEST_F(BrowserTest, FileProtocol) {
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

TEST_F(BrowserTest, LargeFileProtocol) {
  // Serve the pug.js, which should be large enough.
  base::FilePath exe_path;
  PathService::Get(base::FILE_EXE, &exe_path);
  base::FilePath file = exe_path.DirName().DirName().DirName()
                                .Append(FILE_PATH_LITERAL("scripts"))
                                .Append(FILE_PATH_LITERAL("libs"))
                                .Append(FILE_PATH_LITERAL("pug.js"));
  std::string content;
  ASSERT_TRUE(base::ReadFileToString(file, &content));
  nu::Browser::RegisterProtocol("large", [&](const std::string& url) {
    return new nu::ProtocolStringJob(
        "text/html",
        "<html><body><script id='s'>" + content + "</script></body></html>");
  });
  browser_->on_finish_navigation.Connect([&content](nu::Browser* browser,
                                                    const std::string& url) {
    std::string command = base::StringPrintf(
        "document.getElementById('s').textContent.length == %d",
        static_cast<int>(content.size()));
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
