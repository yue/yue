#!/usr/bin/env node

// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, execSync} = require('./common')

const path = require('path')
const fs = require('fs-extra')
const useTmpDir = require('use-tmp-dir')

useTmpDir((cwd) => {
  execSync('node scripts/create_typescript_declarations.js')
  execSync(`git clone https://github.com/yue/node-gui "${cwd}"`)
  execSync('npm install', {cwd})
  fs.copySync('out/Dist/index.d.ts', path.join(cwd, 'index.d.ts'))
  execSync('npm pack', {cwd})
})
