const path = require('path')
const modulePath = path.resolve(__dirname, '..', process.argv[2], 'gui.node')

const gui = require(modulePath)

const win = gui.Window.create({})
win.close()

if (process.versions.electron)
  require('electron').app.quit()
