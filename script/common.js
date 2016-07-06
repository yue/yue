// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

const path = require('path')
const {execSync} = require('child_process')

process.chdir(path.dirname(__dirname))
process.on('uncaughtException', () => process.exit(1))

const ninja = path.join('tools', 'build', process.platform, 'ninja')
const gn = path.join('tools', 'build', process.platform, 'gn')

if (process.platform === 'win32') {
  ninja += '.exe'
  gn += '.exe'
}

const execSyncWrapper = (command, options = { stdio: 'inherit' }) => {
  return execSync(command, options)
}

module.exports = {
  ninja: ninja,
  gn: gn,
  execSync: execSyncWrapper
}
