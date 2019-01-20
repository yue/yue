---
priority: 100
description: How to use Yue with Lua.
---

# Getting started

Lua support in Yue is currently experimental, there are two modes provided:

* A standalone runtime of Lua 5.3 that ships Yue as built-in module;
* A loadable module built for Lua 5.3.

They can be downloaded in the [Releases](https://github.com/yue/yue/releases)
page, the standalone runtime is in the `yue_runtime_VERSION_PLATFORM_ARCH.zip`
archive, the loadable module is in the `lua_yue_LUA-VERSION_VERSION_PLATFORM_ARCH.zip`
archive.

There are plans on making Yue available in LuaRocks.

## Using Yue

Currently only the GUI part of Yue has been provided with Lua bindings, which
can be loaded via `require('yue.gui')`.

```lua
local gui = require('yue.gui')

-- Create window and show it.
local win = gui.Window.create{}
win.onclose = function() gui.MessageLoop.quit() end
win:setcontentview(gui.Label.create('Content View'))
win:setcontentsize{width=400, height=400}
win:center()
win:activate()

-- Enter message loop.
gui.MessageLoop.run()
```
