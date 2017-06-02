#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {execSync} = require('./common')

let targetCpu = 'x64'
if (process.env.TARGET_CPU)
  targetCpu = process.env.TARGET_CPU

execSync(`node ./scripts/bootstrap.js --target-cpu=${targetCpu}`)

const targets = [
  "libnativeui",
  'node_yue',
  'lua_yue',
  'yue',
]
execSync(`node ./scripts/build.js out/Release ${targets.join(' ')}`)

if (targetCpu == 'x64') {
  const tests = [
    'nativeui_unittests',
    'lua_unittests',
  ]
  execSync(`node ./scripts/build.js out/Debug ${tests.join(' ')}`)

  for (test of tests)
    execSync(`./out/Debug/${test} --single-process-tests`)
}
