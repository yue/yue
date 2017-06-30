---
priority: 100
description: How to use Yue in C++.
---

# Getting started

Yue is a bundle of libraries that can be used to create cross platform GUI
programs, currently it includes:

* Chromium's [`base`][base] library for basic cross platform APIs like
  string manipulation and filesystem operations;
* The `nativeui` library for cross platform GUI APIs.

Please note that there is no documentation on the `base` library, you have to
read the header files on how to use it.

Documentation of `nativeui` library is part of this site.

## The `base` library

Though the `base` library is undocumented, there are few guides to help you
learn it:

* [Smart Pointer Guidelines](https://www.chromium.org/developers/smart-pointer-guidelines)
* [`Callback<>` and `Bind()`](https://chromium.googlesource.com/chromium/src/+/master/docs/callback.md)
* [Threading and Tasks in Chrome](https://chromium.googlesource.com/chromium/src/+/master/docs/threading_and_tasks.md)

It is strongly recommended to read all of the guides before using `nativeui`.

## A minimal GUI program using `nativeui` library

```c++
#include "base/command_line.h"
#include "nativeui/nativeui.h"

#if defined(OS_WIN)
int WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  base::CommandLine::Init(0, nullptr);
#else
int main(int argc, const char *argv[]) {
  base::CommandLine::Init(argc, argv);
#endif

  // Initialize the global instance of nativeui.
  nu::State state;

  // Create GUI message loop.
  nu::Lifetime lifetime;

  // Create window with default options, and then show it.
  scoped_refptr<nu::Window> window(new nu::Window(nu::Window::Options()));
  window->SetContentView(new nu::Label("Hello world"));
  window->SetContentSize(nu::SizeF(400, 400));
  window->Center();
  window->Activate();

  // Quit when window is closed.
  window->on_close.Connect([](nu::Window*) {
    nu::Lifetime::GetCurrent()->Quit();
  });

  // Enter message loop.
  lifetime.Run();

  return 0;
}
```

## Building the program with prebuilt static library

There is no assumption on which build system you should use, to use Yue, you can
download the `libyue_VERSION_PLATFORM_ARCH.zip` archive from the
[Releases][releases] page, which includes header files and a prebuilt static
library from of Yue.

Then you can direct your build system to link with Yue and its dependencies.

### Linux

To link with Yue, your program must be linked with GTK3 and X11, and compiled
with at least C++11 enabled:

```bash
clang++ main.cc libyue.a -Iinclude \
        -ldl `pkg-config --cflags --libs gtk+-3.0 x11` \
        -D_GLIBCXX_USE_CXX11_ABI=0 \
        -std=c++14 -DNDEBUG \
        -fdata-sections -ffunction-sections -Wl,--gc-section
```

The `NDEBUG` is required to be defined, because the prebuilt binaries were built
with Release configuration, and your program should use the same configuration.

The `_GLIBCXX_USE_CXX11_ABI=0` is required to be defined, because the prebuilt
binaries were compiled with clang targeting Debian Jessie, and we need to avoid
using cxx11 ABI introduced in newer versions of libstdc++.

In order to minimize the size of binary, `--gc-section` should be passed to the
linker to remove unused code.

### macOS

To link with Yue, your program must be linked with `AppKit.framework` and
`Security.framework`, and compiled with at least C++11 enabled:

```bash
clang++ main.cc libyue.a -Iinclude \
        -framework AppKit -framework Security \
        -std=c++14 -DNDEBUG -Wl,-dead_strip
```

The `NDEBUG` is required to be defined, because the prebuilt binaries were built
with Release configuration, and your program should use the same configuration.

In order to minimize the size of binary, `-Wl,-dead_strip` should be passed to
the linker to remove unused code.

### Windows

To link with Yue, you program must:

* Be a Win32 project;
* Use the `Release` configuration;
* Use `/MT` (static Release version) in `Runtime Library` of `Code Generation`;
* Have `NOMINMAX` defined in `Preprocessor Definitions`;
* Have the `include` directory of the downloaded archive added to the `Include
  Directories`;
* Have the path of the downloaded archive added to the `Library Directories`;
* Have `setupapi.lib;powrprof.lib;ws2_32.lib;dbghelp.lib;shlwapi.lib;version.lib;winmm.lib;psapi.lib;dwmapi.lib;gdi32.lib;gdiplus.lib;libyue.lib`
  added to the `Additional Dependencies` of the `Linker`;
* Have `setupapi.dll;powrprof.dll;dwmapi.dll` added to the `Delay Loaded DLLs` of
  the `Linker`.

[base]: https://chromium.googlesource.com/chromium/src/base/
[releases]: https://github.com/yue/yue/releases
