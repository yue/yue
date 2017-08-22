// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_CHROME_VIEW_MAC_H_
#define NODE_YUE_CHROME_VIEW_MAC_H_

#include <node.h>

#include "nativeui/nativeui.h"

namespace node_yue {

class ChromeView : public nu::View {
 public:
  explicit ChromeView(v8::Local<v8::Value> buf);

  // View class name.
  static const char kClassName[];

  // View:
  const char* GetClassName() const override;

 protected:
  ~ChromeView() override;
};

}  // namespace node_yue

#endif  // NODE_YUE_CHROME_VIEW_MAC_H_
