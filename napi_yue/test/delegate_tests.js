exports.runTests = (gui, assert, {runInNewScope, gcUntil}) => {
  const win = gui.Window.create({})

  assert.strictEqual(win.shouldClose, null, 'delegate defaults to null')
  assert.throws(() => {
    win.shouldClose = 1
  }, /conversion failure from Number to Function/)

  const shouldClose = () => {}
  win.shouldClose = shouldClose
  assert.strictEqual(win.shouldClose, shouldClose, 'delegate cache')

  win.shouldClose = null
  assert.strictEqual(win.shouldClose, null, 'delegate reset')
}
