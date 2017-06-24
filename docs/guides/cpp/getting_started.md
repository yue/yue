---
priority: 100
description: How to use Yue in C++.
---

# Getting started

Yue is a bundle of libraries that can be used to create cross platform GUI
programs, currently it includes:

* Chromium's [`base`][base] library for basic cross platform APIs like
  string manipulation and filesystem operations;
* And the `nativeui` library for cross platform GUI APIs.

Please notice that there is no documentation on the `base` library, you have to
read the header files on how to use it.

Documentation of `nativeui` library is part of this site.

## A minimal program

```c++
#include "base/bind.h"
#include "base/command_line.h"
#include "nativeui/nativeui.h"

int main(int argc, const char *argv[]) {
  // Initialize the base library.
  base::CommandLine::Init(argc, argv);

  // Initialize the global instance of nativeui.
  nu::State state;

  // Create GUI message loop.
  nu::Lifetime lifetime;

  // Create window with default options, and then show it.
  scoped_refptr<nu::Window> window(new nu::Window(nu::Window::Options()));
  window->SetContentSize(nu::SizeF(400, 400));
  window->Center();
  window->Activate();

  // Quit when window is closed.
  window->on_close.Connect(base::Bind([](nu::Window*) {
    nu::Lifetime::GetCurrent()->Quit();
  }));

  // Enter message loop.
  lifetime.Run();

  return 0;
}
```

## Building the program with Yue

There is no assumption on which build system you should use, to use Yue, you can
download the `libyue_VERSION_PLATFORM_ARCH.zip` archive from the
[Releases][releases] page, which includes header files and a prebuilt static
library from of Yue.

### macOS

To link with Yue, your program must be linked with `AppKit.framework` and
`Security.framework`, and compiled with at least C++11 enabled:

```bash
clang++ main.cc libyue.a \
        -Iinclude -framework AppKit -framework Security \
        -std=c++14 -DNDEBUG -Wl,-dead_strip
```

Since the prebuilt binaries were built in Release configuration, you have to
define `NDEBUG` to make sure your program uses the same STL configuration.

You should also pass `-Wl,-dead_strip` to the linker to remove unused code, so
the generated binary would have minimal size.

[base]: https://chromium.googlesource.com/chromium/src/base/
[releases]: https://github.com/yue/yue/releases
