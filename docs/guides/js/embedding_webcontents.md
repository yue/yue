---
priority: 50
description: How to embed a WebContents of Electron in Yue's window
---

# Embedding WebContents of Electron

With Electron 1.8.x on macOS, it is possible to embed Electron's `WebContents`
in Yue's window as a a `View`. But note that it is just a proof of concept, you
should not use it in production.

The key is to get the native view of `WebContents` with
`WebContents.getNativeView()` API, and then wrap it with Yue's `ChromeView`.

```js
let page
electron.app.once('ready', () => {
  page = electron.webContents.create({isBrowserView: true})
  const chrome = gui.ChromeView.create(page.getNativeView())
  win.setContentView(chrome)
})
```

The full example can be found at
https://github.com/yue/yue-sample-apps/tree/master/electron_webcontents.
