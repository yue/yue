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
* [Chromium C++ style guide](https://source.chromium.org/chromium/chromium/src/+/master:styleguide/c++/c++.md)
* [Threading and Tasks](https://source.chromium.org/chromium/chromium/src/+/master:docs/threading_and_tasks.md)
* [Callback<> and Bind()](https://source.chromium.org/chromium/chromium/src/+/master:docs/callback.md)

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

  // Intialize GUI toolkit.
  nu::Lifetime lifetime;

  // Initialize the global instance of nativeui.
  nu::State state;

  // Create window with default options, and then show it.
  scoped_refptr<nu::Window> window(new nu::Window(nu::Window::Options()));
  window->SetContentView(new nu::Label("Hello world"));
  window->SetContentSize(nu::SizeF(400, 400));
  window->Center();
  window->Activate();

  // Quit when window is closed.
  window->on_close.Connect([](nu::Window*) {
    nu::MessageLoop::Quit();
  });

  // Enter message loop.
  nu::MessageLoop::Run();

  return 0;
}
```

## Building the program from source code

There is no assumption on which build system you should use. To use Yue, you can
download the `libyue_VERSION_PLATFORM.zip` archive from the [Releases][releases]
page, which includes header and source files of Yue.

The `libyue` zip archive includes following files:

* `include` - Headers of `nativeui` and `base` libraries.
* `src`
  * `mac` - Source code for macOS platform.
  * `linux` - Source code for Linux platform.
  * `win` - Source code for Win32 platform.
* `sample_app` - A sample app using Yue.
* `CMakeLists.txt` - An example cmake file building the sample app.

Note that since `base` and `nativeui` are very complex codebases,
implementations for different platforms have been split into different files,
and it would cause headaches when building them directly with build systems
other than `GN`.

So in order to make it simple to embed Yue into other build systems, the
`libyue` zip archives include processed source code files: each platform gets
its own source code archives, which can be added to any build system directly
without worrying about adding special compiling rules.

If you are working on a cross-platform app and want to build for all platforms,
you can simply download all the source code archives and merge them together.
The C++ files for each platform are organized into different subdirectories
under `src`, and you only need to add rules to tell your build system to build
each subdirectory on each platform.

### CMake

The `CMakeLists.txt` shipped in the `libyue` zip archives provides an example
of building apps with Yue from source code using CMake. To try it:

1. [Download CMake](https://cmake.org/download/).

2. Enter the directory of libyue and create a `build` sub-directory for
   building.

    ```bash
    cd libyue_VERSION_PLATFORM
    mkdir build
    cd build
    ```

3. Generate project and then build.

   __macOS__

   ```bash
   cmake .. -G Xcode
   open Yue.xcodeproj
   ```

   __Linux__

   ```bash
   cmake .. -D CMAKE_BUILD_TYPE=Release
   make
   ```

   __Windows__

   ```bash
   cmake .. -G "Visual Studio 16 2019" -A x64
   start Yue.sln
   ```

### Reuse the `CMakeLists.txt`

The provided `CMakeLists.txt` file is also designed to be reused by existing
CMake projects directly:

```cmake
add_subdirectory(libyue)
target_link_libraries(YourApp Yue)
```

A live example can be found at https://github.com/yue/muban.

### Notes on WebView2

Currently building with WebView2 on Windows requires installing the
[Microsoft.Web.WebView2](https://www.nuget.org/packages/Microsoft.Web.WebView2)
nuget package, and CMake does not have very good support for nuget packages
for now.

So WebView2 support is disabled by default in libyue, to compile with support
for WebView2, you have to manually add the `WEBVIEW2_SUPPORT` to defines, and
then reference the WebView2 nuget package in the Visual Studio solution
yourself.

Until there is better support of nuget in CMake, no further documentation on
building with WebView2 will be provided.

[base]: https://chromium.googlesource.com/chromium/src/base/
[releases]: https://github.com/yue/yue/releases
