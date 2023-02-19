exports.runTests = (gui, assert) => {
  const win = gui.Window.create({})

  assert.throws(() => {
    win.onClose = 1
  }, /conversion failure from Number to Function/)

  assert.strictEqual(win.onClose, win.onClose, 'signal cache')

  const signalTable = gui.getAttachedTable(win.onClose)
  assert.equal(signalTable.get('name'), 'onClose')
  assert.equal(signalTable.get('responder'), win)

  const table = gui.getAttachedTable(win)
  assert.equal(table.size, 0)

  const id = win.onClose.connect(() => {})
  const slots = table.get('slots').get('onClose')
  assert.equal(slots.size, 1)
  assert.notEqual(slots.get(id), null)

  const callback = () => {}
  win.onClose = callback
  assert.equal(slots.size, 2)
  assert.strictEqual(slots.get(2), callback)

  win.onClose.disconnect(id)
  assert.equal(slots.size, 1)
  win.onClose.disconnectAll()
  assert.equal(table.get('slots').get('onClose'), null)
}
