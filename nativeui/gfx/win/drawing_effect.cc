// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/drawing_effect.h"

#include <shlwapi.h>

namespace nu {

DrawingEffect::DrawingEffect() {}

DrawingEffect::~DrawingEffect() {}

HRESULT __stdcall DrawingEffect::QueryInterface(const IID& riid,
                                                void** ppvObject) {
  const QITAB QITable[] = {
    QITABENT(DrawingEffect, IUnknown),
    {0},
  };
  return QISearch(this, QITable, riid, ppvObject);
}

ULONG __stdcall DrawingEffect::AddRef() {
  return InterlockedIncrement(&ref_);
}

ULONG __stdcall DrawingEffect::Release() {
  auto cref = InterlockedDecrement(&ref_);
  if (cref == 0)
    delete this;
  return cref;
}

}  // namespace nu
