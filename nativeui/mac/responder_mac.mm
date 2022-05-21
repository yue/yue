// Copyright 2022 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/responder.h"

#include "nativeui/mac/events_handler.h"
#include "nativeui/mac/mouse_capture.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_responder.h"

namespace nu {

namespace {

// There is no way to know when another application has installed an event
// monitor, we have to assume only current app can capture view.
Responder* g_captured_responder = nullptr;

}  // namespace

void Responder::SetCapture() {
  if (g_captured_responder)
    g_captured_responder->ReleaseCapture();

  NUPrivate* priv = [GetNative() nuPrivate];
  priv->mouse_capture.reset(new MouseCapture(this));
  g_captured_responder = this;
}

void Responder::ReleaseCapture() {
  if (g_captured_responder != this)
    return;

  NUPrivate* priv = [GetNative() nuPrivate];
  priv->mouse_capture.reset();
  g_captured_responder = nullptr;
  on_capture_lost.Emit(this);
}

bool Responder::HasCapture() const {
  return g_captured_responder == this;
}

void Responder::PlatformInstallMouseClickEvents() {
  AddMouseClickEventHandler(GetNative());
}

void Responder::PlatformInstallMouseMoveEvents() {
  [GetNative() enableTracking];
  AddMouseMoveEventHandler(GetNative());
}

void Responder::PlatformInstallKeyEvents() {
  AddKeyEventHandler(GetNative());
}

}  // namespace nu
