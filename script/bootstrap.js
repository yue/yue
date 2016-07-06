#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

const path = require('path')
const {execSync} = require('child_process')

process.chdir(path.dirname(__dirname))
process.on('uncaughtException', () => process.exit(1))

const gn = process.platform === 'win32' ? 'gn.exe' : 'gn'

if (process.platform !== 'win32') {
  execSync('python tools/clang/scripts/update.py')
}

execSync('git submodule sync --recursive')
execSync('git submodule update --init --recursive')
execSync(`tools/build/${process.platform}/${gn} gen out/Default`, {
  stdio: 'inherit'
})
