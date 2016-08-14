// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_NATIVEUI_EXPORT_H_
#define NATIVEUI_NATIVEUI_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(NATIVEUI_IMPLEMENTATION)
#define NATIVEUI_EXPORT __declspec(dllexport)
#else
#define NATIVEUI_EXPORT __declspec(dllimport)
#endif  // defined(NATIVEUI_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(NATIVEUI_IMPLEMENTATION)
#define NATIVEUI_EXPORT __attribute__((visibility("default")))
#else
#define NATIVEUI_EXPORT
#endif
#endif

#else  // defined(COMPONENT_BUILD)
#define NATIVEUI_EXPORT
#endif

#endif  // NATIVEUI_NATIVEUI_EXPORT_H_
