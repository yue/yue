# Build instructions

In Yue to make development easier, we have some Node.js scripts to do common
building tasks. Under the hood Yue is a project using [GN][gn] for
configuration, and [ninja][ninja] for building.

It is recommended to understand [GN][gn] and [ninja][ninja], and read Yue's
build scripts before actually building Yue.

## System requirements

* x64 operating system
* Git
* Python 2.7 (used by [GN][gn])
* Node.js 10 or later (used by Yue's build scripts)

### Windows

* Visual Studio 2019
  * "Desktop development with C++" component
  * Latest Windows 10 SDK

### Linux

* `webkit2gtk` library >= 2.8
* GCC >= 6

### macOS

* macOS >= 10.15
* Xcode >= 11
  * macOS 10.15 SDK

## Bootstrapping

After checking out code, it is necessary to download dependencies and generate
project files.

```
node scripts/bootstrap.js
```

The [ninja][ninja] build files are generated under `out` directory, you can use
the `ninja` command to build Yue, or you can just make use of the build scripts.

## Building

Running the build script without arguments would build default targets with the
`out/Component` configuration. Default targets are listed in `BUILD.gn`.

```
node scripts/build.js
```

You can also pass a target to the build script.

```
node scripts/build.js nativeui_unittests
```

You can also pass the configuration to build with, the actual parameters of each
configuration can be found in `scripts/bootstrap.js`.

```
node scripts/build.js out/Debug nativeui_unittests
```

### Building Node.js native modules

By default building the `node_yue` target would build the Node.js native module
for the Node.js installed on your system, to build for other versions and
runtimes, you can use the `scripts/create_node_extension.js` script.

```
node scripts/create_node_extension.js {runtime} {version}
```

Currently we only support using `node` and `electron` as `runtime`.

### Generating documentaions

The documentaions of Yue are written in markdown and stored in YAML, the HTML
versions for different languages are automatically generated under `out/Dist`.

```
node scripts/create_docs.js
```

[gn]: https://chromium.googlesource.com/chromium/src/+/master/tools/gn/README.md
[ninja]: https://ninja-build.org
