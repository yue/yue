#!/usr/bin/env node

// Copyright 2021 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {execSync, spawnSync} = require('./common')

const path = require('path')
const useTmpDir = require('use-tmp-dir')

const runtime = process.argv[2]
const version = process.argv[3]
if (!runtime || !version) {
  console.error('Usage: test_node_extension runtime version')
  process.exit(1)
}

useTmpDir((tmpDir) => {
  if (runtime == 'electron')
    testElectron(tmpDir)
  else if (runtime == 'node')
    testNode(tmpDir)
  else
    throw new Error(`Unrecognized runtime ${runtime}`)
}).catch((error) => {
  // On Windows it may fail to remove dir due to Electron's process not exiting.
  if (error.code == 'ENOTEMPTY')
    console.error(error)
  else
    throw error
})

function testElectron(cwd) {
  execSync(`npm install electron@${version}`, {cwd})
  const electron = require(path.join(cwd, 'node_modules', 'electron'))
  spawnSync(electron, ['node_yue/smoke_test.js', 'out/Node'])
}

function testNode(cwd) {
  execSync(`npm install node@${version}`, {cwd})
  let node = path.join(cwd, 'node_modules', 'node', 'bin', 'node')
  if (process.platform == 'win')
    node += '.exe'
  spawnSync(node, ['node_yue/smoke_test.js', 'out/Node'])
}
