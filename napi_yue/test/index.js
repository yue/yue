const path = require('path')
const fs = require('fs')
const {assert} = require('../../third_party/bundled_node_modules/chai')

const modulePath = path.resolve(__dirname, '..', '..', process.argv[2], 'gui.node')
const gui = require(modulePath)

if (process.versions.electron) {
  const electron = require('electron')
  electron.app.once('ready', () => {
    main().then(() => electron.app.quit())
  })
} else {
  main().then(() => process.exit(0))
}

async function main() {
  try {
    for (const f of fs.readdirSync(__dirname)) {
      if (!f.endsWith('_tests.js'))
        continue
      await require(path.join(__dirname, f)).runTests(gui, assert, {runInNewScope, gcUntil})
    }
  } catch (e) {
    console.log(e)
    process.exit(1)
  }
}

async function runInNewScope(func) {
  await (async function() {
    await func()
  })()
}

function gcUntil(condition) {
  return new Promise((resolve, reject) => {
    let count = 0
    function gcAndCheck() {
      setImmediate(() => {
        count++
        gc()
        if (condition()) {
          resolve()
        } else if (count < 10) {
          gcAndCheck()
        } else {
          reject('GC failure')
        }
      })
    }
    gcAndCheck()
  })
}
