// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_BROWSER_H_
#define NATIVEUI_BROWSER_H_

#include <map>
#include <string>

#include "base/values.h"
#include "nativeui/util/function_caller.h"
#include "nativeui/view.h"

namespace nu {

class NATIVEUI_EXPORT Browser : public View {
 public:
  using ExecutionCallback = std::function<void(bool, base::Value)>;
  using BindingFunc = std::function<void(base::Value)>;

  Browser();

  // View class name.
  static const char kClassName[];

  // View:
  const char* GetClassName() const override;

  void LoadURL(const std::string& url);
  void ExecuteJavaScript(const std::string& code,
                         const ExecutionCallback& callback);

  void SetBindingName(const std::string& name);
  void AddRawBinding(const std::string& name, const BindingFunc& func);
  void RemoveBinding(const std::string& name);

  // Automatically deduce argument types.
  template<typename Sig>
  void AddBinding(const std::string& name, const std::function<Sig>& func) {
    AddRawBinding(name, [func](base::Value args) {
      internal::Dispatcher<Sig>::DispatchToCallback(func, args);
    });
  }

  // Events.
  Signal<void(Browser*)> on_close;
  Signal<void(Browser*)> on_finish_navigation;

  // Private: Called from web pages to invoke native bindings.
  void InvokeBindings(const std::string& name, base::Value args);

  // Private:
  const std::string& binding_name() const { return binding_name_; }
  const std::map<std::string, BindingFunc>& bindings() const {
    return bindings_;
  }

 protected:
  ~Browser() override;

 private:
  void UpdateBindings();

  std::string binding_name_;
  std::map<std::string, BindingFunc> bindings_;
};

}  // namespace nu

#endif  // NATIVEUI_BROWSER_H_
