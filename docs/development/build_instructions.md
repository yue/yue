# Build instructions

These the instructions for building Yue.

## System requirements

* x64 machine.
* Git.
* Python 2.7 (used by [GN](https://chromium.googlesource.com/chromium/src/+/master/tools/gn/README.md)).
* Node.js 8 or later (used by our build scripts).

### Windows

* Visual Studio 2017 with Update 3.x.
  * "Desktop development with C++" component.
  * 10.0.15063 Windows 10 SDK.

### Linux

On Linux you have to use a recent distribution with following dependencies:

* `webkit2gtk` library >= 2.8.
* GCC >= 6.

### macOS

* macOS >= 10.12.
* Xcode >= 8.
* OS X 10.12 SDK.

## Bootstrapping

Download necessary toolcahins and generate project files, you only need to run
the script once after you have pulled the source code:

```
node scripts/bootstrap.js
```

## Building

Running the build script without arguments would build default targets with the
`out/Component` configuration. Default targets are listed in `BUILD.gn`.

```
node scripts/build.js
```

You can pass the target to the build script to only build it.

```
node scripts/build.js nativeui_unittests
```

You can also pass the configuration to build with, the build parameters of each
configuration can be found in `scripts/bootstrap.js`.

```
node scripts/build.js out/Debug nativeui_unittests
```

### Building Node.js native modules

By default building the `node_yue` target would build the Node.js native module
specified for the Node.js installed on your system, to build for other versions
and runtimes, you can use the `scripts/create_node_extension.js` script.

```
node scripts/create_node_extension.js {runtime} {version}
```

Currently we only support using `electron` and `node` as `runtime`.

### Generating documentaions

The documentaions of Yue are written markdown and stored in YAML, the HTML
versions for different languages are automatically generated under `out/Dist`.

```
node scripts/create_docs.js
```

## Creating distribution

After building with `out/Debug` and `out/Release` configurations, distributions
in ZIP archives can be created.

```
node scripts/create_dist.js
```
