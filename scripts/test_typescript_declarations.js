#!/usr/bin/env node

// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, execSync} = require('./common')

const os = require('os')
const path = require('path')
const fs = require('fs-extra')

// Our work dir.
const zipname = `node_yue_types_${version}`
const cwd = path.join(os.tmpdir(), zipname)
try {
  fs.removeSync(cwd)
} catch {}
fs.ensureDirSync(cwd)

// Print working dir when quit unexpected.
process.removeAllListeners('uncaughtException')
process.on('uncaughtException', (error) => {
  console.error('Working dir:', cwd)
  console.error(error)
  process.exit(1)
})

execSync('node scripts/create_typescript_declarations.js')
execSync(`git clone https://github.com/yue/node-gui "${cwd}"`)
fs.copySync('out/Dist/index.d.ts', path.join(cwd, 'index.d.ts'))
execSync('npm install', {cwd})
execSync('npm pack', {cwd})

// Cleanup.
fs.removeSync(cwd)
