// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_VALUE_CONVERSION_H_
#define NATIVEUI_MAC_VALUE_CONVERSION_H_

namespace base {
class Value;
}

namespace nu {

base::Value NSValueToBaseValue(id value);

}  // namespace nu

#endif  // NATIVEUI_MAC_VALUE_CONVERSION_H_
