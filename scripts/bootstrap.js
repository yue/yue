#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {argv, execSync, spawnSync} = require('./common')

if (process.platform !== 'win32') {
  execSync('python tools/clang/scripts/update.py')
}
if (process.platform === 'linux') {
  // TODO(zcbenz): Support more arch.
  execSync('python build/linux/sysroot_scripts/install-sysroot.py --arch amd64')
  execSync('node scripts/update_gold.js')
}

execSync('git submodule sync --recursive')
execSync('git submodule update --init --recursive')
execSync(`node scripts/download_node_headers.js node ${process.version}`)

let target_cpu = 'x64'
for (let arg of argv) {
  if (arg.startsWith('--target_cpu='))
    target_cpu = arg.substr(arg.indexOf('=') + 1)
}

gen('out/Debug', [
  'is_component_build=true',
  'is_debug=true',
  'use_sysroot=false',
  'node_runtime="node"',
  `node_version="${process.version}"`,
  `target_cpu="${target_cpu}"`,
])
gen('out/Release', [
  'is_component_build=false',
  'is_debug=false',
  'is_official_build=true',
  'node_runtime="node"',
  `node_version="${process.version}"`,
  `target_cpu="${target_cpu}"`,
])

function gen(dir, args) {
  spawnSync('gn', ['gen', dir, `--args=${args.join(' ')}`])
}
