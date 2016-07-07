#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

const https = require('https')
const fs = require('fs')
const path = require('path')
const {spawn, spawnSync} = require('child_process')

require('./common')

// Get revision.
const update = path.join('tools', 'clang', 'scripts', 'update.py')
const {stdout} = spawnSync('python', [update, '--print-revision'])
const revision = stdout.toString().trim()

// Download and untar.
const target = path.join('third_party', 'llvm-build', 'Release+Asserts')
const url = `https://commondatastorage.googleapis.com/chromium-browser-clang/Linux_x64/llvmgold-${revision}.tgz`
const untar = spawn('tar', ['x'], {cwd: target})
https.get(url, (response) => response.pipe(untar.stdin))
