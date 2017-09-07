---
priority: 100
description: How to use Yue with Node.js and Electron.
---

# Getting started

Yue is a library for creating native GUI programs, by using its V8 bindings you
can easily build a desktop app with Node.js.

## Installing

The V8 bindings of Yue exists as the `gui` npm module.

### Node.js

Installing Yue in Node.js is as easy as:

```bash
npm i gui
```

But note that `gui` is a native module, you have to reinstall it whenever you
change Node.js versions or platforms.

Only Node.js v7 and above are supported.

### Electron

For Electron you must follow the [Using Native Node Modules][native-module]
guide on installing the `gui` module:

```bash
# Runtime is electron.
export npm_config_runtime=electron
# Electron's version.
export npm_config_target=1.8.0
# The architecture of Electron, can be ia32, arm or x64.
export npm_config_arch=x64
# Install the module, and store cache to ~/.electron-gyp.
HOME=~/.electron-gyp npm i gui
```

Only Electron v1.6.x and above are supported.

## Using Yue

Like other Node.js modules, Yue can be used with a simple `require('gui')`, but
depending on the runtime you use, there are some extra work to make GUI message
loop run.

### Node.js

Because Node.js was not designed for desktop apps, it is using libuv event loop
that is not compatible with GUI message loops.

In order to make GUI work in Node.js, you have to take over the control of event
loop by using the `Lifetime` API, which would run native GUI message loop while
still handling libuv events:

```js
const gui = require('gui')

if (!process.versions.yode) {
  gui.lifetime.run()  // block until gui.lifetime.quit() is called
  process.exit(0)
}
```

To quit the message loop, you can call the `lifetime.quit()` API, which would
break the blocking `lifetime.run()` call and continue the script, usually you
should exit the process after that.

Note that this hack does not work perfectly, you should never use it in
production.

#### Yode

To solve the problem of incompatible event loop, you can use the
[Yode](https://github.com/yue/yode) project as a replacement of Node.js. Yode
is a fork of Node.js that replaces libuv event loop with native GUI message
loops, so there would be no need to use the `lifetime.run()` hack.

Unlike Node.js which would quit the process when there is no work to do, the
processes of Yode would keep running forever, until you call the
`lifetime.quit()` API to quit current message loop.

After quitting the GUI message loop, the libuv event loop is still running, and
the process will exit when all pending Node.js requests have finished.

The code example above also showed how to make the script run under both Yode
and Node.js.

### Electron

Since the main process of Electron uses GUI message loops, there is no need to
use the `Lifetime` API, and the `Lifetime` API of Yue is not available when
running under Electron. You should always use the `app` API of Electron to
manage the process's lifetime.

```js
const gui = require('gui')
const electron = require('electron')

electron.app.once('ready', () => {
  // Do work here...
})
```

## Example: Text editor

This example shows how to create windows and views in Yue, and how to manage
their layout.

## Example: Float widget

This example shows how to use frameless window and how to draw things.

## More

* [FAQ](https://github.com/yue/help#faq)

[native-module]: https://github.com/electron/electron/blob/master/docs/tutorial/using-native-node-modules.md
