// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const fs = require('fs')
const path = require('path')
const {execSync, spawnSync} = require('child_process')

// Switch to root dir.
process.chdir(path.dirname(__dirname))

// We are not using toolchain from depot_tools.
process.env.DEPOT_TOOLS_WIN_TOOLCHAIN = 0

// Expose ninja and gn to PATH.
const binaries_dir = path.resolve('tools', 'build', process.platform)
process.env.PATH = `${binaries_dir}${path.delimiter}${process.env.PATH}`

// Get yue's version.
const version = String(execSync('git describe --always --tags')).trim()

// Get target_cpu from args.gn.
let targetCpu = 'x64'
if (fs.existsSync('out/Release/args.gn')) {
  const content = String(fs.readFileSync('out/Release/args.gn'))
  const match = content.match(/target_cpu = "(.*)"/)
  if (match && match.length > 1)
    targetCpu = match[1]
}

// Get target OS.
const targetOs = {
  win32: 'win',
  linux: 'linux',
  darwin: 'mac',
}[process.platform]

// Parse args.
let verbose = false
const argv = process.argv.slice(2).filter((arg) => {
  if (arg == '-v' || arg == '--verbose') {
    verbose = true
    return false
  } else if (arg.startsWith('--target-cpu=')) {
    targetCpu = arg.substr(arg.indexOf('=') + 1)
    return false
  } else {
    return true
  }
})

// Make dir and ignore error.
function mkdir(dir) {
  if (fs.existsSync(dir)) return
  mkdir(path.dirname(dir))
  fs.mkdirSync(dir)
}

// Helper around execSync.
const execSyncWrapper = (command, options = {}) => {
  // Print command output by default.
  if (!options.stdio)
    options.stdio = 'inherit'
  // Merge the custom env to global env.
  if (options.env)
    options.env = Object.assign(options.env, process.env)
  return execSync(command, options)
}

const spawnSyncWrapper = (exec, args, options = {}) => {
  // Print command output by default.
  if (!options.stdio)
    options.stdio = 'inherit'
  // Merge the custom env to global env.
  if (options.env)
    options.env = Object.assign(options.env, process.env)
  return spawnSync(exec, args, options)
}

// Don't log out Node.js stack trace.
if (!verbose) {
  process.on('uncaughtException', (error) => {
    console.error('Exit with error:', error.message)
    process.exit(1)
  })
}

// Export public APIs.
module.exports = {
  verbose,
  version,
  argv,
  targetCpu,
  targetOs,
  mkdir,
  execSync: execSyncWrapper,
  spawnSync: spawnSyncWrapper,
}
