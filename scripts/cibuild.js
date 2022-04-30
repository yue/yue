#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {targetCpu, targetOs, hostCpu, execSync} = require('./common')
const {nodeVersions, electronVersions, luaVersions} = require('./config')

const path = require('path')
const fs = require('fs-extra')

// Mark this is CI build.
process.env.CI = 'true'

// Bootstrap.
execSync(`node ./scripts/bootstrap.js --target-cpu=${targetCpu}`)

// Build common targets.
execSync('node ./scripts/build.js out/Release')
execSync('node ./scripts/build.js out/Debug')

// Run test except for cross compilation.
if ((targetCpu == hostCpu) || (targetOs == 'win' && targetCpu == 'x86')) {
  const tests = [
    'nativeui_unittests',
    'lua_unittests',
    'lua_yue_unittests',
  ]
  execSync(`node ./scripts/build.js out/Debug ${tests.join(' ')}`)
  for (test of tests)
    execSync(`${path.join('out', 'Debug', test)}`)
}

// Test node modules can load.
if (targetCpu == 'x64') {
  for (const config of ['Release', 'Debug'])
    execSync(`node node_yue/smoke_test.js out/${config}`)
}

fs.emptyDirSync('out/Dist')

// Test source code distributions.
if ((targetCpu == hostCpu) || (targetOs == 'win' && targetCpu == 'x86'))
  execSync(`node ./scripts/test_libyue.js`)

// Create docs, but only do it for linux/x64 when running on CI, to avoid
// uploading docs for multiple times.
if (process.env.CI != 'true' || (targetOs == 'linux' && targetCpu == 'x64')) {
  execSync('node ./scripts/create_docs.js')
  execSync(`node ./scripts/test_typescript_declarations.js`)
}

// Build lua extensions.
for (const luaver of luaVersions)
  execSync(`node ./scripts/create_lua_extension.js --target-cpu=${targetCpu} lua ${luaver}`)

// Build node extensions.
if (targetOs == 'win' && targetCpu.startsWith('arm'))
  process.exit(0)
const runtimes = {
  node: nodeVersions,
  electron: electronVersions,
}
for (let runtime in runtimes) {
  for (let nodever of runtimes[runtime])
    execSync(`node ./scripts/create_node_extension.js --target-cpu=${targetCpu} ${runtime} ${nodever}`)
}
