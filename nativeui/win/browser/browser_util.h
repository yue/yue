// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_BROWSER_BROWSER_UTIL_H_
#define NATIVEUI_WIN_BROWSER_BROWSER_UTIL_H_

#include "base/values.h"
#include "nativeui/win/util/dispatch_invoke.h"

namespace nu {

// Set register key to prevent using compatible mode of IE.
void FixIECompatibleMode();

// Convert VARIANT to base::Value.
base::Value VARIANTToValue(IDispatchEx* script,
                           const base::win::ScopedVariant& value);

}  // namespace nu

#endif  // NATIVEUI_WIN_BROWSER_BROWSER_UTIL_H_
