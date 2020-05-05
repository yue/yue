---
priority: 100
description: How to use Yue with Lua.
---

# Getting started

Currently Lua support in Yue is experimental, and a loadable module built
for Lua 5.3 is provided which can be downloaded from the
[Releases](https://github.com/yue/yue/releases) page. The filename is
`lua_yue_LUA-VERSION_VERSION_PLATFORM_ARCH.zip`.

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
