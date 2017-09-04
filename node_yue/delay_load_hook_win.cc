// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include <windows.h>

#include <delayimp.h>
#include <string.h>

namespace {

FARPROC WINAPI LoadHook(unsigned int event, DelayLoadInfo* info) {
  HMODULE m;
  if (event != dliNotePreLoadLibrary)
    return NULL;

  if (_stricmp(info->szDll, "node.exe") != 0 &&
      _stricmp(info->szDll, "yode.exe") != 0 &&
      _stricmp(info->szDll, "node.dll") != 0)
    return NULL;

  m = ::GetModuleHandle(NULL);
  return reinterpret_cast<FARPROC>(m);
}

}  // namespace

decltype(__pfnDliNotifyHook2) __pfnDliNotifyHook2 = LoadHook;
