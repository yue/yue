---
priority: 98
description: Walkthrough of the event system.
---
# Events and delegates

## Event

Yue uses the signal/slot pattern for the event system, each event is a signal
that can be connected by multiple slots, and event handlers are slots that can
connect to multiple signals.

In Yue each signal is an instance of [`Signal`](../api/signal.html) class,
while slot is just a function.

```lua
local gui = require('yue.gui')
gui.app.onready:connect(function() print('on ready') end)
```

It is also possible to connect a slot to a signal by assignment, which is a
shorthand of calling `connect`.

```lua
function app.onready()
  print('on ready')
end
```

### Preventing the default behavior

Certain events have default behaviors that can be prevented.

Returning `true` means preventing the default behavior, and other slots of the
event will not be executed. While returning nothing or `false` means the slot
is only observing the event.

```lua
local gui = require('yue.gui')
local view = gui.Container.create()
view.onmousedown:connect(function(self, event) return true end)
```

## Delegate

Unlike event that can have multiple handlers connected, a delegate is a single
`function` that can only be assigned by one function.

```lua
local gui = require('yue.gui')
local window = gui.Window.create{}
window.shouldclose = function(self) return false end
```

The delegates are usually used over events when the library is requesting data
dynamically.
