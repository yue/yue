---
priority: 99
description: How to package your app.
---

# App packaging

While you can let users install your app via `npm install`, it is recommended to
package your app and ship platform specific app bundles, so users can get best
desktop experience.

Yode supports appending ASAR archive to the executable and starting from it, so
for apps using Yue the best way to package is using [`yackage`][yackage], which
can automatically put the app into ASAR archive and concatenate it to Yode.

By packaging the app you can also gain extra benefits like avoiding long path
names problem on Windows, and speeding up the `require` calls.

## Using `yackage`

The README of [`yackage`][yackage] has detailed usage instructions, basically
you can generate independent app bundle under `out/` directory with:

```
yackage build out/
```

## Limitations of Node APIs

Due to the low-level nature of the Node APIs, there are some limitations when
using Node APIs after you packaged your app, details can be found at
https://electronjs.org/docs/tutorial/application-packaging#limitations-of-the-node-api.

## Limitations of Yue APIs

Yue is essentially a thin wrapper of native APIs, which do not recognize ASAR
archives. So the APIs of Yue can not read files inside ASAR archives, i.e., can
not read your app's files when packaged.

Currently you can solve this by using the `--unpack` flag to exclude the assets
of your app when using `yackage`, and in your app's code use `fs.realpath` to
get the filesystem path of the assets before passing the path to Yue APIs.

For example you can unpack the images when using `yackage`:

```
yackage dist out --unpack "+(*.node|*.png)"
```

And in app's code call `fs.realpath` before reading the images:

```js
const p = fs.realpathSync(path.join(__dirname, 'image.png'))
const image = gui.Image.createFromPath(p)
```

[yackage]: https://github.com/yue/yackage
