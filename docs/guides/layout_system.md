---
priority: 90
description: How to manage the layout of Views.
---

# Layout system

Yue uses [Facebook Yoga](https://facebook.github.io/yoga/) as layout engine,
which provides Flexbox style layout system.

Yue does not support CSS itself, it only uses the concept of Flexbox for layout,
and you have to manually set style for each View.

```cpp
view->SetStyle("flex-direction", "column", "flex", 1);
```


```lua
view:setstyle{flexDirection='column', flex=1}
```


```js
view.setStyle({flexDirection: 'column', flex: 1})
```

It should be noted that not all CSS properties are supported, and there is no
plan for support of tables, floats, or similar CSS concepts.

## View and Container

In Yue all widgets are inherited from the virtual class `View`, which represents
a leaf node in the layout system. The `Container` is a `View` that can have
multiple child `View`s, in the layout system the child `View`s of `Container`
are treated as child nodes.

There are `View`s that can have content view like `Group` or `Scroll`, but their
content views are treated as root nodes in layout system, instead of being child
nodes of their parents.

Following code is an example of list view.

```cpp
scoped_refptr<nu::Scroll> list_view(new nu::Scroll);
scoped_refptr<nu::Container> content_view(new nu::Container());
content_view->SetStyle("flex-direction", "column");

for (size_t i = 0; i < 100; ++i) {
  scoped_refptr<nu::Label> item(new nu::Label(base::IntToString(i)));
  unsigned g = 155. / 100 * i + 100;
  item->SetBackgroundColor(nu::Color(100, g, 100));
  content_view->AddChildView(item.get());
}

list_view->SetContentSize(content_view->GetPreferredSize());
list_view->SetContentView(content_view.get());
```


```lua
local listview = gui.Scroll.create()
local contentview = gui.Container.create()
contentview:setstyle{flexdirection='column'}

for i = 1, 100 do
  local item = gui.Label.create(i)
  local g = math.floor(155 / 100 * i + 100)
  item:setbackgroundcolor(gui.Color.rgb(100, g, 100))
  contentview:addchildview(item)
end

listview:setcontentsize(contentview:getpreferredsize())
listview:setcontentview(contentview)
```


```js
const listView = gui.Scroll.create()
const contentView = gui.Container.create()
contentView.setStyle({flexDirection: 'column'})

for (let i = 0; i < 100; ++i) {
  const item = gui.Label.create(String(i))
  const g = Math.floor(155 / 100 * i + 100)
  item.setBackgroundColor(gui.Color.rgb(100, g, 100))
  contentView.addChildView(item)
}

listView.setContentSize(contentView.getPreferredSize())
listView.setContentView(contentView)
```

## Style properties

For a complete list of supported style properties, it is recommended to read the
[documentation of Yoga](https://yogalayout.com/docs).
