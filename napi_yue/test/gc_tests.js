exports.runTests = async (gui, assert, {gcUtil, runInNewScope}) => {
  if (!process.argv.includes('--run-gc-tests'))
    return
  if (!gc)
    throw new Error('GC tests require --expose-gc flag')
  if (!gui.MessageLoop.run)
    throw new Error('GC tests require running with official node')

  runInNewScope(() => {
    const win = gui.Window.create({})
    win.onClose = () => win.isClosed()
    gui.addFinalizer(win, gui.MessageLoop.quit)
  })

  const interval = setInterval(gc, 100)
  const timeout = setTimeout(() => {
    throw new Error('GC test failed')
  }, 10 * 60 * 1000)

  gui.MessageLoop.run()

  clearTimeout(timeout)
  clearInterval(interval)
}
