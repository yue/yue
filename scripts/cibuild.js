#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {targetCpu, targetOs, execSync} = require('./common')

const path = require('path')

// Mark this is CI build.
process.env.CI = 'true'

// Lint code.
execSync('node ./scripts/cpplint.js')

// Bootstrap.
execSync(`node ./scripts/bootstrap.js --target-cpu=${targetCpu}`)

// Run test except for cross compilation on Linux.
if (targetOs != 'linux' || targetCpu == 'x64') {
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

// Test  distributions.
execSync(`node ./scripts/test_libyue.js`)

// Build node extensions.
const runtimes = {
  node: [
    '10.14.1',
    '11.4.0',
  ],
  electron: [
  ],
}
for (let runtime in runtimes) {
  for (let nodever of runtimes[runtime])
    execSync(`node ./scripts/create_node_extension.js --target-cpu=${targetCpu} ${runtime} ${nodever}`)
}
