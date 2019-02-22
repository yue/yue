---
priority: 80
description: Walkthrough of the drag and drop APIs.
---

# Drag and drop

Like the clipboard, drag and drop is a way to transfer data between different
applications.

Most operating systems implement the drag and drop system in the same way with
clipboard, and in Yue dragged data are represented as instances of the
[`Clipboard::Data`](../api/clipboard_data.html).

For code examples on handling drag and drop, please see the sample app in
[`yue-sample-apps/drag_source`](https://github.com/yue/yue-sample-apps/blob/master/drag_source).

## Drag destination

A `View` can be made a drag destination by using the
[`View.RegisterDraggedTypes`](../api/view.html#registerdraggedtypes-types) API,
which gives the view abilities to accept dropped data.

```cpp
view->RegisterDraggedTypes({nu::Clipboard::Data::Type::Image});
```


```lua
view:registerdraggedtypes({'image'})
```


```js
view.registerDraggedTypes(['image'])
```

When users drag data with registered types to the view, drag related events will
be emitted in sequence:

1. The [`handle_drag_enter`][handle_drag_enter] and
   [`on_drag_leave`][on_drag_leave] will be called when the cursor enters or
   leaves the view while dragging.
2. The [`handle_drag_update`][handle_drag_update] will be called while cursor
   moves in the view while dragging.
3. The [`handle_drop`][handle_drop] will be called user releases cursor and
   drops the data on the view.

To accept dropped data, the [`handle_drag_enter`][handle_drag_enter] should be
implemented first to decide which [`DragOperation`](../api/dragoperation.html)
to perform when cursor is dropped. Depending on the returned drag operation,
the cursor may have different icons.

```cpp
view->handle_drag_enter = [&is_dragging](nu::View*, nu::DraggingInfo*, const nu::PointF&) {
  is_dragging = true;
  return nu::DRAG_OPERATION_COPY;
};
```


```lua
function view.handledragenter(self, info, point)
  isDragging = true
  return gui.DraggingInfo.dragoperationcopy
end
```


```js
view.handleDragEnter = (self, info, point) => {
  isDragging = true
  return gui.DraggingInfo.dragOperationCopy
}
```

If you are implementing some visual effects when there are data being dragged to
the view, you may want to do some cleanup in [`on_drag_leave`][on_drag_leave],
which is called when cursor moves out of the view _and_ before users drops the
data on the view.

```cpp
view->on_drag_leave.Connect([&is_dragging](nu::View*) {
  is_dragging = false;
});
```


```lua
function view.ondragleave(self)
  isDragging = true
end
```


```js
view.onDragLeave = (self) => {
  isDragging = true
}
```

If the view supports different drag operations depending on the cursor position,
you may want to implement [`handle_drag_update`][handle_drag_update], and return
the drag operation according to the cursor position in the view.

If you don't implement [`handle_drag_update`][handle_drag_update], the return
value of previous [`handle_drag_enter`][handle_drag_enter] call will be used.

```cpp
view->handle_drag_update = [](nu::View*, nu::DraggingInfo*, const nu::PointF& point) {
  if (PointInDragArea(point))
    return nu::DRAG_OPERATION_COPY;
  else
    return nu::DRAG_OPERATION_NONE;
};
```


```lua
function view.handledragupdate(self, info, point)
  if (pointInDragArea(point))
    return gui.DraggingInfo.dragoperationcopy
  else
    return gui.DraggingInfo.dragoperationnone
end
```


```js
view.handledragUpdate = (self, info, point) => {
  if (pointInDragArea(point))
    return gui.DraggingInfo.dragOperationCopy
  else
    return gui.DraggingInfo.dragOperationNone
}
```

Finally, by implementing [`handle_drop`][handle_drop], you can read the dragged
data from the passed [`DraggingInfo`](../api/dragginginfo.html) instance.

```cpp
view->handle_drop = [](nu::View*, nu::DraggingInfo*, const nu::PointF& point) {
  nu::Clipboard::Data data = info->GetData(nu::Clipboard::Data::Type::Image);
  if (data.type() == nu::Clipboard::Data::Type::Image) {
    DisplayImage(data.image());
    return true;
  }
  return false;
};
```


```lua
function view.handledrop(self, info, point)
  if (info:isdataavailable('image')) {
    displayImage(info:getdata('image').value)
    return true
  }
  return false
end
```


```js
view.handleDrop = (self, info, point) => {
  if (info.isDataAvailable('image')) {
    displayImage(info.getData('image').value)
    return true
  }
  return false
}
```

## Drag source

To make it possible for users to drag some data from current applications to
other destinations, you can listen to mouse down events and use the
[`View.DoDrag`](../api/view.html#dodrag-data-operations) API.

```cpp
source->SetMouseDownCanMoveWindow(false);

source->on_mouse_down.Connect([&file_path](nu::View* self, const nu::MouseEvent&) {
  std::vector<nu::Clipboard::Data> data;
  data.emplace_back(std::vector<base::FilePath>({file_path}));
  int ret = self->DoDragWithOptions(std::move(data), nu::DRAG_OPERATION_COPY, nu::DragOptions(image.get()));
  if (ret == nu::DRAG_OPERATION_COPY)
    CopyData();
  else if (ret == nu::DRAG_OPERATION_LINK)
    LinkData();
  return true;
});
```


```lua
source:setmousedowncanmovewindow(false)

function source.onmousedown(self)
  local ret = self:doDrag({{type: 'file-paths', value: {filePath}}},
                          gui.DraggingInfo.dragoperationcopy | gui.DraggingInfo.dragoperationlink)
  if (ret == gui.DraggingInfo.dragoperationcopy)
    copyData()
  else if (ret == gui.DraggingInfo.dragoperationlink)
    linkData()
  return true
end
```


```js
source.setMouseDownCanMoveWindow(false)
source.onMouseDown = (self) => {
  const ret = self.doDrag([{type: 'file-paths', value: [filePath]}],
                          gui.DraggingInfo.dragOperationCopy | gui.DraggingInfo.dragOperationLink)
  if (ret == gui.DraggingInfo.dragOperationCopy)
    copyData()
  else if (ret == gui.DraggingInfo.dragOperationLink)
    linkData()
  return true
}
```

The API should be called with the data to provide, and the supported drag
operations. The call will be blocked until the user cancels the dragging or
drops the data, and it will return the drag operation used by the destination.

[handle_drag_enter]: ../api/view.html#handledragenter-self-info-point
[handle_drag_update]: ../api/view.html#handledragupdate-self-info-point
[handle_drop]: ../api/view.html#handledrop-self-info-point
[on_drag_leave]: ../api/view.html#ondragleave-self-info
