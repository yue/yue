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

## 

## Building a program with Yue

There is no assumption on which build system you should use, to use Yue, you can
download the `libyue_VERSION_PLATFORM_ARCH.zip` archive from the
[Releases][releases] page, which includes header files and a prebuilt static
library from of Yue.

### macOS

To link with Yue, your program must be linked with `AppKit.framework` and
`Security.framework`, and compiled with C++14 enabled:

```bash
clang++ main.cc \
        libyue/libyue.a -Ilibyue/include -framework AppKit -framework Security \
        -std=c++14 -Wl,-dead_strip
```

You should also pass `-Wl,-dead_strip` to the linker to remove unused code, so
the generated binary would have minimal size.

[base]: https://chromium.googlesource.com/chromium/src/base/
[releases]: https://github.com/yue/yue/releases
