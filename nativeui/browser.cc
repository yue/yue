// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/browser.h"

#include <utility>

#include "base/base64.h"
#include "base/json/string_escape.h"
#include "base/rand_util.h"
#include "base/strings/stringprintf.h"

#include "nativeui/protocol_file_job.h"

namespace nu {

// static
const char Browser::kClassName[] = "Browser";

Browser::Browser(Options options) {
  PlatformInit(std::move(options));
  // Generate a random number as security key.
  base::Base64Encode(base::RandBytesAsString(16), &security_key_);
}

Browser::~Browser() {
  PlatformDestroy();
}

const char* Browser::GetClassName() const {
  return kClassName;
}

void Browser::SetBindingName(const std::string& name) {
  base::EscapeJSONString(name, false, &binding_name_);
  if (!stop_serving_)
    PlatformUpdateBindings();
}

void Browser::AddRawBinding(const std::string& name, const BindingFunc& func) {
  if (name.empty())
    return;
  std::string escaped;
  base::EscapeJSONString(name, false, &escaped);
  bindings_[escaped] = func;
  if (!stop_serving_)
    PlatformUpdateBindings();
}

void Browser::RemoveBinding(const std::string& name) {
  if (name.empty())
    return;
  std::string escaped;
  base::EscapeJSONString(name, false, &escaped);
  bindings_.erase(escaped);
  if (!stop_serving_)
    PlatformUpdateBindings();
}

bool Browser::InvokeBindings(const std::string& key,
                             const std::string& method,
                             base::Value args) {
  if (stop_serving_)
    return false;
  if (key != security_key_) {
    stop_serving_ = true;
    LOG(ERROR) << "Recevied invalid key, stop serving navite bindings";
    return false;
  }
  auto it = bindings_.find(method);
  if (it == bindings_.end()) {
    LOG(ERROR) << "Invoking invalid method: " << method;
    return false;
  }
  it->second(this, std::move(args));
  return true;
}

std::string Browser::GetBindingScript() {
  std::string code = "(function(key, external, binding) {";
  std::string name = binding_name_;
  if (name.empty()) {
    name = "window";
  } else {
    // window[name] = {};
    name = base::StringPrintf("window[\"%s\"]", name.c_str());
    code = name + " = {};" + code;
  }
  // Insert bindings.
  for (const auto& it : bindings_) {
    code += base::StringPrintf(
        "binding[\"%s\"] = function() {"
        "  var args = Array.prototype.slice.call(arguments);"
#if defined(OS_WIN)
        // On WebKit we can only pass one argument.
        "  external.postMessage(key, \"%s\", JSON.stringify(args));"
#else
        "  external.postMessage([key, \"%s\", args]);"
#endif
        "};",
        it.first.c_str(), it.first.c_str());
  }
  code += base::StringPrintf("})(\"%s\", %s, %s);",
                             security_key_.c_str(),
#if defined(OS_WIN)
                             "window.external",
#else
                             "window.webkit.messageHandlers.yue",
#endif
                             name.c_str());
  return code;
}

}  // namespace nu
