---
priority: 98
description: Walkthrough of the event system.
---

# Events and delegates

The event system of Yue heavily relies on `std::function`, it is strongly
recommended to know more about `std::function` before continue.

## Event

Yue uses the signal/slot pattern for the event system, each event is a signal
that can be connected by multiple slots, and event handlers are slots that can
connect to multiple signals.

In Yue signal is represented as the [`nu::Signal<Sig>`](../api/signal.html)
template class, while slot is represented as the `std::function<Sig>` template
class. `Sig` is the signature of slot.

```cpp
void Main() {
  nu::App::GetCurrent()->on_ready.Connect([] {
    LOG(ERROR) << "OnReady";
  });
}
```

### Preventing the default behavior

Certain events have default behaviors that can be prevented, which require the
slots to return `bool`.

Returning `true` means preventing the default behavior, and other slots of the
event will not be executed. While returning `false` means the slot is only
observing the event.

```cpp
bool OnMouseDown(nu::Responder* self, const nu::MouseEvent& event) {
  return true;
}

void Main(nu::View* view) {
  view->on_mouse_down.Connect(&OnMouseDown);
}
```

## Delegate

Unlike event that can have multiple handlers connected, a delegate is a single
`std::function<Sig>` data member that can only be assigned by one function.

```cpp
void Main(nu::Window* window) {
  window->should_close = [](nu::Responder* self) { return false; };
}
```

The delegates are usually used over events when the library is requesting data
dynamically.

[callback]: https://chromium.googlesource.com/chromium/src/+/master/docs/callback.md
