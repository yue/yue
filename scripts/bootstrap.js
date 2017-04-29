#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {execSync, spawnSync} = require('./common')

if (process.platform !== 'win32') {
  execSync('python tools/clang/scripts/update.py')
}
if (process.platform === 'linux') {
  execSync('python build/linux/sysroot_scripts/install-sysroot.py')
  execSync('node scripts/update_gold.js')
}

execSync('git submodule sync --recursive')
execSync('git submodule update --init --recursive')

gen('out/Debug', [
  'is_component_build=true',
  'is_debug=true',
  'node_runtime="node"',
  `node_version="${process.version}"`,
])
gen('out/Release', [
  'is_component_build=false',
  'is_debug=false',
  'is_official_build=true',
  'node_runtime="node"',
  `node_version="${process.version}"`,
])

function gen(dir, args) {
  spawnSync('gn', ['gen', dir, `--args=${args.join(' ')}`])
}
