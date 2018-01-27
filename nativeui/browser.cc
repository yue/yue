// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#include <memory>
#include <utility>

#include "base/json/json_reader.h"

namespace nu {

// static
const char Browser::kClassName[] = "Browser";

const char* Browser::GetClassName() const {
  return kClassName;
}

void Browser::AddRawBinding(const std::string& name, const BindingFunc& func) {
  bindings_[name] = func;
}

void Browser::RemoveBinding(const std::string& name) {
  bindings_.erase(name);
}

void Browser::OnPostMessage(const std::string& name, const std::string& json) {
  std::unique_ptr<base::Value> pv = base::JSONReader::Read(json);
  if (!pv || !pv->is_list()) {
    LOG(ERROR) << "Invalid message passed: " << json;
    return;
  }
  InvokeBindings(name, std::move(*pv.release()));
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
