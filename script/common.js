// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

const path = require('path')
const {execSync, spawnSync} = require('child_process')

// Switch to root dir.
process.chdir(path.dirname(__dirname))

// Common paths.
const ninja = path.join('tools', 'build', process.platform, 'ninja')
const gn = path.join('tools', 'build', process.platform, 'gn')

if (process.platform === 'win32') {
  ninja += '.exe'
  gn += '.exe'
}

// Print command output by default.
const execSyncWrapper = (command, options = { stdio: 'inherit' }) => {
  return execSync(command, options)
}

// Parse args.
let dir = 'out/Default'
const args = process.argv.slice(2).filter((arg) => {
  if (arg.includes('/') && !arg.startsWith('-')) {
    dir = arg
    return false
  } else {
    return true
  }
})

// Run command and pass args.
const runSync = (command, commandArgs = []) => {
  commandArgs = commandArgs.concat(dir).concat(args)
  return spawnSync(command, commandArgs, { stdio: 'inherit' })
}

// Don't log out Node.js stack trace.
process.on('uncaughtException', () => process.exit(1))

// Export public APIs.
module.exports = { ninja, gn, runSync, execSync: execSyncWrapper }
