---
priority: 100
description: How to use Yue in Node.js and Electron.
---

# Getting started

Yue provides V8 bindings that can be used in Node.js and Electron to create
native GUI. Currently only it is only supported in Node.js v7 and above, and
Electron v1.6.x and above.

## Install Yue

The V8 bindings of Yue can be downloaded by installing the `gui` npm module.

### Node.js

For Node.js it can be simply done with `npm i gui`, but when you switch between
different major versions of Node.js, you have to reinstall the module because
it downloads prebuilt binaries that are targeted for a certain version of
Node.js.

### Electron

For Electron you must follow the [Using Native Node Modules][native-module]
guide on installing `gui` module, because the module downloads prebuilt binaries
that are targeted for different runtimes and versions.

```bash
# Runtime is electron.
export npm_config_runtime=electron
# Electron's version.
export npm_config_target=1.6.0
# The architecture of Electron, can be ia32, arm or x64.
export npm_config_arch=x64
# Install the module, and store cache to ~/.electron-gyp.
HOME=~/.electron-gyp npm i gui
```

## Using Yue

You can use Yue by simply `require('gui')`, but depending on the runtime you
use, you may have to take over the event loop.

### Node.js

Node.js uses libuv for its event loop, which unfortunately does not work together
with GUI programs, so to use Yue in Node.js, you have to take over the control of
event loop by using the `Lifetime` API:

```js
const gui = require('gui')

const win = gui.Window.create({})
win.onClose = () => gui.lifetime.quit()
win.setContentView(gui.Label.create('Content View'))
win.setContentSize({width: 400, height: 400})
win.center()

gui.lifetime.onReady = () => win.activate()

gui.lifetime.run()
process.exit(0)
```

Please note that this feature is not stable (and may never be), so do not use
Yue in Node.js in any production environment.

### Electron

Electron uses GUI message loop as its event loop, so you can just use Yue
without worrying about event loop.

```js
const electron = require('electron')
const gui = require('gui')

const win = gui.Window.create({})
win.onClose = () => gui.lifetime.quit()
win.setContentView(gui.Label.create('Content View'))
win.setContentSize({width: 400, height: 400})
win.center()

electron.app.on('ready', () => {
  win.activate()
})
```

Please note that you can only use Yue in the main process of Electron.

[native-module]: https://github.com/electron/electron/blob/master/docs/tutorial/using-native-node-modules.md
