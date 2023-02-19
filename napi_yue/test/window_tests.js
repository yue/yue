exports.runTests = async (gui) => {
  return new Promise((resolve) => {
    const win = gui.Window.create({})
    win.onClose = () => resolve()
    win.close()
  })
}
