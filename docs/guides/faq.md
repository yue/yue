---
priority: 101
description: Frequently asked questions.
---

# FAQ

### What does Yue mean?

It means moon in Chinese.

### Why a new GUI library? / What's the differences between Yue and XXX?

When writing GUI programs, I had been searching for a library that:

* Uses native widgets;
* Works on all major desktop platforms;
* Has a modern and clean C++ interface;
* Has good support for High DPI;
* Uses windowless controls on Windows;
* Generates small executable size;
* With friendly license for closed source apps.

There were lots of good GUI toolkits, but I could not find one that meets all above conditions, so I decided to write my own.

### Do I need to open source my project when using Yue?

Unless you have modified Yue's source code and linked your project with Yue statically, there is no requirements on which license your project should use.

### Window disappears after some time when using Yode.

You might notice `Window` or `Tray` instances disappearing after running for some time when using Yode, this is usually because the JavaScript objects of those instances got garbage collected.

```js
const win = gui.Window.create({})
win.activate()
if (!process.versions.yode && !process.versions.electron) {
  gui.MessageLoop.run()
  process.exit(0)
}
```

When using Node.js to run code, the whole program will block at the `gui.MessageLoop.run()` call, and the local variables defined in the main script will not be garbage collected, i.e. the `win` object will live until the program quits.

However when switching to Yode, the execution of the main script will finish immediately, and the local variables will be garbage collected as usual, i.e. the `win` object will be garbage collected after some time and the window will disappear.

For simple apps you can fix it by declaring the main window as a global variable.

```js
global.mainWindow = gui.Window.create({})
mainWindow.activate()
```

For multi-window apps it is recommended to manage the windows with a global map.

```js
global.windowManager = new Set()
const win = gui.Window.create({})
windowManager.add(win)
win.onClose = () => windowManager.delete(win)
mainWindow.activate()
```

### What's the minimum version of Windows supported?

By using Win32 API and GDI+, Yue can work on Windows >= 7. It is also possible to make Yue work on Windows XP with some efforts, but this is not on my roadmap.

### Why using windowless controls on Windows?

The Win32 Common Controls have a few problems:

* most controls can not be nested;
* with large numbers of controls performance would be slow.
* it is hard/impossible to make controls transparent;
* it is hard/impossible to solve the flickering problem;
* it is hard/impossible to handle controls' mouse and keyboard events;

(Whether it is hard or impossible depends on the types of controls.)

To make the windowless controls still look native, most controls are drawn with the UxTheme API, which should be the same with how Windows itself renders Win32 Common Controls.

However with windowless controls we also lose accessibility by default, my plan is to use the UI Automation provider API to provide accessibility interface for the windowless controls.

### Are all widgets windowless on Windows?

No, currently text input related widgets are still implemented by using Win32 Common Controls, because it is rather tough to implement a text input widget from scratch, and I have to focus on more important things first.

In future I'll make all widgets windowless, but it might be optional since it may bloat the size of executable.

### Why using GDI+ instead of Direct2D?

Because I was more familiar with GDI+, I'll migrate the painting code to Direct2D in future.

Note that most controls are drawn with UxTheme instead of GDI+, so unless you are doing very heavy custom animation with the drawing API, there is no need to worry about the performance of GDI+.
