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
* Friendly license for closed source apps.

There were lots of good GUI toolkits, but I could not find one that meets all above conditions, so I decided to write my own.

### Do I need to open source my project when using Yue?

Unless you have modified Yue's source code and linked your project with Yue statically, there is no requirements on which license your project should use.

### Your website looks shabby.

I have put priority on the documentation site, I'll improve the homepage in future.

### Will Yue support XAML/QML?

No, Yue will always be a widgets library, there is no plan to implement high level languages like XAML/QML in Yue. The goal of Yue is to provide a low-level library that can be used to easily implement things like Rect Native and XAML/QML.

However Yue will support creating widgets from simple XML descriptions, since it is essential for writing a visual GUI builder.

### Will Yue support CSS?

No.

### What's the minimum version of Windows supported?

By using Win32 API and GDI+, Yue can work on Windows >= 7. It is also possible to make Yue work on Windows XP with some efforts, but this is not on my roadmap.

### Are all widgets windowless on Windows?

No, currently text input related widgets are still implemented by using Win32 Common Controls, because it is rather tough to implement a text input widget from scratch, and I have to focus on more important things first.

In future I'll make all widgets windowless, but it might be optional since it may bloat the size of executable.

### Why using GDI+ instead of Direct2D?

Because I was more familiar with GDI+, I'll migrate the painting code to Direct2D in future.

### Can I embed a web browser in Yue?

Currently there are efforts on supporting native webview in Yue, e.g. `WebView` on macOS, `webkitgtk` on Linux, and Internet Explorer on Windows.

### Can I use Yue in NW.js?

No it is not possible, Yue requires to be used in the UI thread of the main process, which is not supported by NW.js.

[paid-plan]: https://github.com/yue/yue/tree/master/docs/paid_plans
