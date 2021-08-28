---
priority: 100
description: How to use Yue with Lua.
---

# Getting started

Currently Lua support in Yue is experimental, and loadable modules built
for Lua 5.1/5.3/5.4 can be downloaded from the
[Releases](https://github.com/yue/yue/releases) page. The filename is
`lua_yue_lua_$LUAVERSION_$YUEVERSION_$PLATFORM_$ARCH.zip`.

There are plans on making Yue available in LuaRocks.

## Using Yue

Currently only the GUI part of Yue has been provided with Lua bindings, which
can be loaded via `require('yue.gui')`.

```lua
-- Specify where to find yue.gui module.
package.cpath = '/Path/To/Yue/?.dll;/Path/To/Yue/?.so'

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

### Notes on Windows

Due to using GUI related Win32 APIs in Yue, it is recommended to use the
`wlua.exe` binaries to load Yue. The normal `lua.exe` binaries are built with
Console subsystem and may throw DLL errors when using Yue.
