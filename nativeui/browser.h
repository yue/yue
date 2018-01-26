// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_BROWSER_H_
#define NATIVEUI_BROWSER_H_

#include <string>

#include "base/values.h"
#include "nativeui/view.h"

namespace nu {

class NATIVEUI_EXPORT Browser : public View {
 public:
  using ExecutionCallback = std::function<void(bool, base::Value)>;

  Browser();

  // View class name.
  static const char kClassName[];

  // View:
  const char* GetClassName() const override;

  void LoadURL(const std::string& url);
  void ExecuteJavaScript(const std::string& code,
                         const ExecutionCallback& callback);

  // Events.
  Signal<void(Browser*)> on_close;
  Signal<void(Browser*)> on_finish_navigation;

 protected:
  ~Browser() override;
};

}  // namespace nu

#endif  // NATIVEUI_BROWSER_H_
