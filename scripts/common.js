// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const path = require('path')
const {execSync, spawnSync} = require('child_process')

// Switch to root dir.
process.chdir(path.dirname(__dirname))

// We are not using toolchain from depot_tools.
process.env.DEPOT_TOOLS_WIN_TOOLCHAIN = 0

// Expose ninja and gn to PATH.
const binaries_dir = path.resolve('tools', 'build', process.platform)
process.env.PATH = `${binaries_dir}${path.delimiter}${process.env.PATH}`

// Parse args.
let verbose = false
process.argv = process.argv.slice(2).filter((arg) => {
  if (arg === '-v' || arg === '--verbose') {
    verbose = true
    return false
  } else {
    return true
  }
})

// Print command output by default.
const execSyncWrapper = (command, options = { stdio: 'inherit' }) => {
  return execSync(command, options)
}

// Don't log out Node.js stack trace.
if (!verbose) {
  process.on('uncaughtException', (error) => {
    console.error('Exit with error:', error.message)
    process.exit(1)
  })
}

// Export public APIs.
module.exports = { verbose, execSync: execSyncWrapper }
