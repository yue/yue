// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_UTIL_DISPATCH_INVOKE_H_
#define NATIVEUI_WIN_UTIL_DISPATCH_INVOKE_H_

#include <dispex.h>

#include "base/win/scoped_bstr.h"
#include "base/win/scoped_variant.h"

namespace nu {

inline bool Invoke(IDispatchEx* script,
                   const wchar_t* name,
                   WORD flags,
                   base::win::ScopedVariant* result) {
  base::win::ScopedBstr name_str(name);
  DISPID name_id;
  if (FAILED(script->GetDispID(name_str.Get(), 0, &name_id)))
    return false;
  DISPPARAMS disp_params = { nullptr, nullptr, 0, 0 };
  if (FAILED(script->InvokeEx(name_id, LOCALE_USER_DEFAULT, flags, &disp_params,
                              result->Receive(), nullptr, nullptr)))
    return false;
  return true;
}

template<typename... ArgTypes>
bool Invoke(IDispatchEx* script,
            const wchar_t* name,
            WORD flags,
            base::win::ScopedVariant* result,
            const ArgTypes&... args) {
  base::win::ScopedBstr name_str(name);
  DISPID name_id;
  if (FAILED(script->GetDispID(name_str.Get(), 0, &name_id)))
    return false;
  VARIANT vars[sizeof...(args)] = { args... };
  DISPPARAMS disp_params = { vars, nullptr, sizeof...(args), 0 };
  if (FAILED(script->InvokeEx(name_id, LOCALE_USER_DEFAULT, flags, &disp_params,
                              result->Receive(), nullptr, nullptr)))
    return false;
  return true;
}

}  // namespace nu

#endif  // NATIVEUI_WIN_UTIL_DISPATCH_INVOKE_H_
