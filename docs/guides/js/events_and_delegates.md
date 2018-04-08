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

```js
const {app} = require('gui')
app.onReady.connect(() => console.log('on ready'))
```

It is also possible to connect a slot to a signal by assignment, which is a
shorthand of calling `connect`.

```js
app.onReady = () => console.log('on ready')
```

### Preventing the default behavior

Certain events have default behaviors that can be prevented.

Returning `true` means preventing the default behavior, and other slots of the
event will not be executed. While returning nothing or `false` means the slot
is only observing the event.

```js
const {Container} = require('gui')
let view = Container.create()
view.onMouseDown.connect((self, event) => true)
```

## Delegate

Unlike event that can have multiple handlers connected, a delegate is a single
`Function` that can only be assigned by one function.

```js
const {Window} = require('gui')
let window = Window.create({})
window.shouldClose = (self) => false
```

The delegates are usually used over events when the library is requesting data
dynamically.
