#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

const path = require('path')
const {execSync} = require('child_process')

process.chdir(path.dirname(__dirname))
process.on('uncaughtException', () => process.exit(1))

const ninja = process.platform === 'win32' ? 'ninja.exe' : 'ninja'

execSync(`buildtools/${process.platform}/${ninja} -C out/Default`, {
  stdio: 'inherit'
})
