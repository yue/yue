// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_BROWSER_H_
#define NATIVEUI_BROWSER_H_

#include <map>
#include <string>
#include <utility>

#include "base/values.h"
#include "nativeui/protocol_job.h"
#include "nativeui/util/function_caller.h"
#include "nativeui/view.h"

namespace nu {

class NATIVEUI_EXPORT Browser : public View {
 public:
  using ProtocolHandler = std::function<ProtocolJob*(std::string)>;
  using ExecutionCallback = std::function<void(bool, base::Value)>;
  using BindingFunc = std::function<void(Browser*, base::Value)>;

  struct Options {
    bool devtools = false;
    bool context_menu = false;
#if defined(OS_MAC) || defined(OS_LINUX)
    bool allow_file_access_from_files = false;
#endif
#if defined(OS_LINUX)
    bool hardware_acceleration = true;
#endif
#if defined(OS_WIN) && defined(WEBVIEW2_SUPPORT)
    bool webview2_support = false;
    // Used for testing the creation failure of WebView2.
    bool webview2_force_ie = false;
#endif
  };

  explicit Browser(Options options);

  // View class name.
  static const char kClassName[];

  // Protocol APIs.
  static bool RegisterProtocol(const std::string& scheme,
                               ProtocolHandler handler);
  static void UnregisterProtocol(const std::string& scheme);

  // View:
  const char* GetClassName() const override;

#if defined(OS_WIN)
  bool IsWebView2() const;
#endif
  void LoadURL(const std::string& url);
  void LoadHTML(const std::string& html, const std::string& base_url);
  std::string GetURL();
  std::string GetTitle();
  void SetUserAgent(const std::string& user_agent);
#if defined(OS_MAC)
  bool IsMagnifiable() const;
  void SetMagnifiable(bool magnifiable);
#endif
  void ExecuteJavaScript(const std::string& code,
                         const ExecutionCallback& callback);

  void GoBack();
  bool CanGoBack() const;
  void GoForward();
  bool CanGoForward() const;
  void Reload();
  void Stop();
  bool IsLoading() const;

  void SetBindingName(const std::string& name);
  void AddRawBinding(const std::string& name, BindingFunc func);
  void RemoveBinding(const std::string& name);
  bool HasBindings() const;

  // Automatically deduce argument types.
  template<typename Sig>
  void AddBinding(const std::string& name, std::function<Sig> func) {
    AddRawBinding(name, [func = std::move(func)](nu::Browser* browser,
                                                 base::Value args) {
      internal::Dispatcher<Sig>::DispatchToCallback(
          func, browser, std::move(args));
    });
  }
  // Automatically convert function pointer to std::function.
  template<typename T>
  void AddBinding(const std::string& name, T func) {
    using RunType = typename internal::FunctorTraits<T>::RunType;
    AddBinding(name, std::function<RunType>(func));
  }

  // Internal: Called from web pages to invoke native bindings.
  bool InvokeBindings(const std::string& json_arg);

  // Internal: Generate the user script to inject bindings.
  std::string GetBindingScript();

  // Internal: Access to bindings properties.
  bool stop_serving() const { return stop_serving_; }

#if defined(OS_WIN) && defined(WEBVIEW2_SUPPORT)
  std::function<void()>& pending_load() { return pending_load_; }
#endif

  // Events.
  Signal<void(Browser*)> on_close;
  Signal<void(Browser*)> on_update_command;
  Signal<void(Browser*)> on_change_loading;
  Signal<void(Browser*, std::string)> on_update_title;
  Signal<void(Browser*, std::string)> on_start_navigation;
  Signal<void(Browser*, std::string)> on_commit_navigation;
  Signal<void(Browser*, std::string, int)> on_fail_navigation;
  Signal<void(Browser*, std::string)> on_finish_navigation;

 protected:
  ~Browser() override;

#if defined(OS_MAC)
  // Responder:
  void PlatformInstallMouseClickEvents() override;
  void PlatformInstallMouseMoveEvents() override;
#endif

 private:
  void PlatformInit(Options options);
  void PlatformDestroy();
  void PlatformUpdateBindings();

  // Prevent malicous calls to native bindings.
  std::string security_key_;
  bool stop_serving_ = false;

#if defined(OS_WIN) && defined(WEBVIEW2_SUPPORT)
  // The pending op when LoadURL is called before webview is ready.
  std::function<void()> pending_load_;
#endif

  std::string binding_name_;
  std::map<std::string, BindingFunc> bindings_;
};

}  // namespace nu

#endif  // NATIVEUI_BROWSER_H_
