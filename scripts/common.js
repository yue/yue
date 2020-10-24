// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const fs = require('fs')
const path = require('path')
const https = require('https')
const {execSync, spawnSync} = require('child_process')

// Switch to root dir.
process.chdir(path.dirname(__dirname))

// We are not using toolchain from depot_tools.
process.env.DEPOT_TOOLS_WIN_TOOLCHAIN = 0

// Allow searching modules from libs/.
module.parent.paths.push(path.resolve(__dirname, '..', 'third_party', 'bundled_node_modules'))

// Expose ninja and gn to PATH.
const gnDir = path.resolve('building', 'tools', 'gn')
process.env.PATH = `${gnDir}${path.delimiter}${process.env.PATH}`

// Get yue's version.
const version = String(execSync('git describe --always --tags')).trim()

// Get target OS.
const targetOs = {
  win32: 'win',
  linux: 'linux',
  darwin: 'mac',
}[process.platform]

// Get target_cpu from args.gn.
let targetCpu = 'x64'
let clang = targetOs != 'win'
if (fs.existsSync('out/Release/args.gn')) {
  const content = String(fs.readFileSync('out/Release/args.gn'))
  const match = content.match(/target_cpu = "(.*)"/)
  if (match && match.length > 1)
    targetCpu = match[1]
  if (content.includes('is_clang = true'))
    clang = true
  else
    clang = false
}

// Parse args.
let verbose = false
const argv = process.argv.slice(2).filter((arg) => {
  if (arg == '-v' || arg == '--verbose') {
    verbose = true
    return false
  } else if (arg == '--clang') {
    clang = true
    return false
  } else if (arg == '--no-clang') {
    clang = false
    return false
  } else if (arg.startsWith('--target-cpu=')) {
    targetCpu = arg.substr(arg.indexOf('=') + 1)
    return false
  } else {
    return true
  }
})

// The common build configurations.
const config = [
  `target_cpu="${targetCpu}"`,
  'use_allocator="none"',
  'use_allocator_shim=false',
  'use_partition_alloc=false',
  'fatal_linker_warnings=false',
]
if (clang) {
  config.push('is_clang=true',
              'clang_update_script="//building/tools/update-clang.py"')
} else {
  config.push('is_clang=false')
}
if (targetOs == 'mac') {
  config.push('mac_deployment_target="10.10.0"',
              'use_xcode_clang=true')
  if (targetCpu == 'arm64')
    config.push('mac_sdk_min="11.0"')
  else
    config.push('mac_sdk_min="10.15"')
} else if (targetOs == 'linux') {
  // Required for loading dynamic modules.
  config.push('use_cfi_icall=false')
}

function strip(file) {
  // TODO(zcbenz): Use |file| command to determine type.
  if (!file.endsWith('.so') &&
      !file.endsWith('.node') &&
      path.basename(file) != 'yue')
    return
  // TODO(zcbenz): Copy the debug symbols out before striping.
  let strip = 'strip'
  if (targetCpu == 'arm')
    strip = 'arm-linux-gnueabihf-strip'
  else if (targetCpu == 'arm64')
    strip = 'aarch64-linux-gnu-strip'
  spawnSyncWrapper(strip, [file])
}

// Helper to download an URL.
const download = (url, callback, log=true) => {
  https.get(url, (response) => {
    if (log) {
      process.stdout.write(`Downloading ${url} `)
    }
    if (response.statusCode == 302) {
      download(response.headers.location, callback, false)
      return
    }
    let length = 0
    response.on('end', () => {
      if (length > 0)
        process.stdout.write('.')
      console.log(' Done')
    })
    .on('data', (chunk) => {
      length += chunk.length
      while (length >= 1024 * 1024) {
        process.stdout.write('.')
        length %= 1024 * 1024
      }
    })
    callback(response)
  })
}

// Return all the files recursivley.
function searchFiles(dir, suffix, list = []) {
  return fs.readdirSync(dir).reduce((list, filename) => {
    const p = path.join(dir, filename)
    const stat = fs.statSync(p)
    if (stat.isFile() && filename.endsWith(suffix))
      list.push(p)
    else if (stat.isDirectory())
      searchFiles(p, suffix, list)
    return list
  }, list)
}

// Turn stream into Promise.
function streamPromise(stream) {
  return new Promise((resolve, reject) => {
    stream.on('end', () => {
      resolve('end')
    })
    stream.on('finish', () => {
      resolve('finish')
    })
    stream.on('error', (error) => {
      reject(error)
    })
  })
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
  const result = spawnSync(exec, args, options)
  if (result.error)
    throw result.error
  return result
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
  clang,
  config,
  argv,
  targetCpu,
  targetOs,
  strip,
  download,
  searchFiles,
  streamPromise,
  execSync: execSyncWrapper,
  spawnSync: spawnSyncWrapper,
}
