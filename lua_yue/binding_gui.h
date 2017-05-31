// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef LUA_YUE_BINDING_GUI_H_
#define LUA_YUE_BINDING_GUI_H_

#include "lua/lua.h"

#ifdef _WIN32
# define LUA_MODULE_EXPORT __declspec(dllexport)
#else
# define LUA_MODULE_EXPORT __attribute__((visibility("default")))
#endif

extern "C" LUA_MODULE_EXPORT int luaopen_yue_gui(lua::State* state);

#endif  // LUA_YUE_BINDING_GUI_H_
