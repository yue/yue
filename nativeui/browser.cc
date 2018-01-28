// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#include <utility>

#include "base/json/string_escape.h"

namespace nu {

// static
const char Browser::kClassName[] = "Browser";

const char* Browser::GetClassName() const {
  return kClassName;
}

void Browser::SetBindingName(const std::string& name) {
  base::EscapeJSONString(name, false, &binding_name_);
  UpdateBindings();
}

void Browser::AddRawBinding(const std::string& name, const BindingFunc& func) {
  if (name.empty())
    return;
  std::string escaped;
  base::EscapeJSONString(name, false, &escaped);
  bindings_[escaped] = func;
  UpdateBindings();
}

void Browser::RemoveBinding(const std::string& name) {
  if (name.empty())
    return;
  std::string escaped;
  base::EscapeJSONString(name, false, &escaped);
  bindings_.erase(escaped);
  UpdateBindings();
}

void Browser::InvokeBindings(const std::string& method, base::Value args) {
  auto it = bindings_.find(method);
  if (it == bindings_.end()) {
    LOG(ERROR) << "Invoking invalid method: " << method;
    return;
  }
  it->second(std::move(args));
}

}  // namespace nu
