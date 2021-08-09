const path = require('path')
const modulePath = path.resolve(__dirname, '..', process.argv[2], 'gui.node')

const gui = require(modulePath)

if (process.versions.electron) {
  const electron = require('electron')
  electron.app.once('ready', () => {
    test()
    require('electron').app.quit()
  })
} else {
  test()
}

function test() {
  const win = gui.Window.create({})
  win.close()
}
