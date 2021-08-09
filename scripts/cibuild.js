#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {targetCpu, targetOs, execSync} = require('./common')
const {nodeVersions, electronVersions} = require('./config')

const path = require('path')

// Mark this is CI build.
process.env.CI = 'true'

// Lint code.
execSync('node ./scripts/cpplint.js')

// Bootstrap.
execSync(`node ./scripts/bootstrap.js --target-cpu=${targetCpu}`)

// Run test except for cross compilation.
if ((targetCpu == 'x64') || (targetOs == 'win' && targetCpu == 'x86')) {
  const tests = [
    'nativeui_unittests',
    'lua_unittests',
    'lua_yue_unittests',
  ]
  execSync(`node ./scripts/build.js out/Debug ${tests.join(' ')}`)
  for (test of tests)
    execSync(`${path.join('out', 'Debug', test)}`)
}

// Build common targets.
execSync('node ./scripts/build.js out/Release')
execSync('node ./scripts/build.js out/Debug')
execSync('node scripts/create_dist.js')

// Test distributions.
if ((targetCpu == 'x64') || (targetOs == 'win' && targetCpu == 'x86'))
  execSync(`node ./scripts/test_libyue.js`)
// Test typescript types.
if (targetCpu == 'x64' && targetOs == 'linux')
  execSync(`node ./scripts/test_typescript_declarations.js`)
// Test node modules can load.
if (targetCpu == 'x64') {
  for (const config of ['Release', 'Debug'])
    execSync(`node node_yue/smoke_test.js out/${config}`)
}

// Build node extensions.
if (targetOs == 'win' && targetCpu.startsWith('arm'))
  process.exit(0)
const runtimes = {
  node: nodeVersions,
  electron: electronVersions,
}
for (let runtime in runtimes) {
  for (let nodever of runtimes[runtime]) {
    execSync(`node ./scripts/create_node_extension.js --target-cpu=${targetCpu} ${runtime} ${nodever}`)
    if (targetCpu == 'x64')
      execSync(`node ./scripts/test_node_extension.js ${runtime} ${nodever}`)
  }
}
