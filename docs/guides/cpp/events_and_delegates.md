---
priority: 99
description: Walkthrough of the event system.
---

# Events and delegates

The event system of Yue heavily relies on Chromium's `base::Callback` library,
it is recommended to have a read of [Callback and Bind][callback] before
continue.

## Event

Yue uses the signal/slot pattern for the event system, each event is a signal
that can be connected by multiple slots, and event handlers are slots that can
connect to multiple signals.

In Yue signal is represented as the [`nu::Signal<Sig>`](../api/signal.html)
template class, while slot is represented as the
[`base::Callback<Sig>`](../api/base_callback.html) template class. `Sig` is the
signature of slot.

```cpp
void OnReady() {
  LOG(ERROR) << "OnReady";
}

void Main() {
  nu::App::GetCurrent()->on_ready.Connect(base::Bind(&OnReady));
}
```

### Preventing the default behavior

Certain events have default behaviors that can be prevented, which require the
slots to return `bool`.

Returning `true` means preventing the default behavior, and other slots of the
events will not be executed. While returning `false` means the slot is only
observing the event.

```cpp
bool OnMouseDown(nu::View* self, const nu::MouseEvent& event) {
  return true;
}

void Main(nu::View* view) {
  view->on_mouse_down.Connect(base::Bind(&OnMouseDown));
}
```

## Delegate

Unlike event that can have multiple handlers connected, a delegate is a single
`base::Callback<Sig>` that can only be assigned by one function.

```cpp
bool ShouldClose(nu::Window* self) {
  return false;
}

void Main(nu::Window* window) {
  window->should_close = base::Bind(&OnMouseDown);
}
```

[callback]: https://chromium.googlesource.com/chromium/src/+/master/docs/callback.md
