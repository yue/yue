#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {execSync} = require('./common')

execSync(`node ./scripts/bootstrap.js --target-cpu=${process.env.TARGET_CPU}`)

const targets = [
  'node_yue',
  'lua_yue',
  'yue',
]
execSync(`node ./scripts/build.js out/Release ${targets.join(' ')}`)

const tests = [
  'nativeui_unittests',
  'lua_unittests',
]
execSync(`node ./scripts/build.js out/Debug ${tests.join(' ')}`)

for (test of tests)
  execSync(`./out/Debug/${test} --single-process-tests`)
