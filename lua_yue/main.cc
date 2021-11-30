// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "base/command_line.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "lua_yue/builtin_loader.h"
#include "nativeui/lifetime.h"
#include "nativeui/state.h"

int main(int argc, const char *argv[]) {
  // Initialize base library.
  base::CommandLine::Init(argc, argv);

  auto* cmd = base::CommandLine::ForCurrentProcess();
  if (cmd->GetArgs().size() != 1) {
    fprintf(stderr, "Usage: yue <path-to-script>\n");
    return 1;
  }

  // Create lua environment.
  lua::ManagedState state;
  if (!state) {
    fprintf(stderr, "Unable to create lua state\n");
    return 1;
  }

  // Load builtin libraries in lua environment.
  luaL_openlibs(state);
  yue::InsertBuiltinModuleLoader(state);

  // Lua only accepts UTF-8 strings.
  // FIXME(zcbenz): Write our own luaL_loadfile.
#if defined(OS_WIN)
  std::string filename = base::WideToUTF8(cmd->GetArgs()[0]);
#else
  std::string filename = cmd->GetArgs()[0];
#endif

  // Load the main script.
  if (luaL_loadfile(state, filename.c_str()) != LUA_OK ||
      !lua::PCall(state, nullptr)) {
    std::string error;
    lua::Pop(state, &error);
    fprintf(stderr, "Error when running script: %s\n", error.c_str());
    return 1;
  }

  return 0;
}
