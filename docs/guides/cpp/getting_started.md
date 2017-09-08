---
priority: 100
description: How to use Yue in C++.
---

# Getting started

Yue is a bundle of libraries that can be used to create cross platform GUI
programs, currently it includes:

* Chromium's [`base`][base] library for basic cross platform APIs like
  string manipulation and filesystem operations;
* The `nativeui` library for cross platform GUI.

Please note that there is no documentation on the `base` library, you have to
read the header files on how to use it.

Documentation of `nativeui` library is part of this site.

## The `base` library

Though the `base` library is undocumented, there are few guides to help you
learn it:

* [Smart Pointer Guidelines](https://www.chromium.org/developers/smart-pointer-guidelines)

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
[Releases][releases] page, which includes header files and prebuilt static
libraries of Yue.

The libyue zip archive includes following files:

* `include` - Headers of `nativeui` and `base` libraries.
* `lib` - Debug and Release builds of static libraries, the filename of Debug
  version ends with a `d`.
* `sample_app` - A sample app using Yue.
* `CMakeLists.txt` - An example cmake file building the sample app.

If you are interested in integrating Yue into a build system, you can look into
the `CMakeLists.txt` on which compilation options should be used.

### Quick start with CMake

CMake can generate projects according to your preferences, this is a quick start
tutorial if you are too lazy to learn CMake.

1. [Download CMake](https://cmake.org/download/).

2. Enter the directory of libyue and create a `build` sub-directory for
   building.

    ```bash
    cd libyue_VERSION_PLATFORM_ARCH
    mkdir build
    cd build
    ```

3. Generate project and then build.

   __macOS__

   ```bash
   cmake .. -G Xcode
   open YueSampleApp.xcodeproj
   ```

   __Linux__

   ```bash
   cmake -D CMAKE_BUILD_TYPE=Release ..
   make
   ./sample_app
   ```

   __Windows__

   Note that on Windows you need to manually speicfy the version of Visual
   Studio and the architecture to build for.

   ```cmd
   cmake .. -G "Visual Studio 14 Win64"
   start YueSampleApp.sln
   ```

   For certain versions of Visual Studio you also need to manually set the
   "sample_app" as startup project.

[base]: https://chromium.googlesource.com/chromium/src/base/
[releases]: https://github.com/yue/yue/releases
