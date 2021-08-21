// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_LUA_MODULE_EXPORT_H_
#define LUA_LUA_MODULE_EXPORT_H_

#ifdef _WIN32
# define LUA_MODULE_EXPORT __declspec(dllexport)
#else
# define LUA_MODULE_EXPORT __attribute__((visibility("default")))
#endif

#endif  // LUA_LUA_MODULE_EXPORT_H_
