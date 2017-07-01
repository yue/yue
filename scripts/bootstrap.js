#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {targetCpu, targetOs, execSync, spawnSync} = require('./common')

// Get the arch of sysroot.
let sysrootArch = {
  x64: 'amd64',
  x86: 'i386',
  arm: 'arm',
  arm64: 'arm64',
}[targetCpu]

if (process.platform !== 'win32') {
  execSync('python tools/clang/scripts/update.py')
}
if (process.platform === 'linux') {
  // TODO(zcbenz): Support more arch.
  execSync(`python build/linux/sysroot_scripts/install-sysroot.py --arch ${sysrootArch}`)
  execSync('node scripts/update_gold.js')
}

execSync('git submodule sync --recursive')
execSync('git submodule update --init --recursive')
execSync(`node scripts/download_node_headers.js node ${process.version}`)

const commonConfig = [
  'use_allocator_shim=false',
  'fatal_linker_warnings=false',
  `target_cpu="${targetCpu}"`,
  'node_runtime="node"',
  `node_version="${process.version}"`,
]
const componentConfig = [
  'is_component_build=true',
  'is_debug=true',
  'use_sysroot=false',
]
const debugConfig = [
  'is_component_build=false',
  'is_debug=true',
  // This flag caused weird compilation errors when building on Linux.
  'enable_iterator_debugging=false',
]
const releaseConfig = [
  'is_component_build=false',
  'is_debug=false',
  // This flag makes the library incompatible with other versions of clang.
  'allow_posix_link_time_opt=false',
]

// Don't set official build for Windows, which increases the size of libyue.
if (targetOs != 'win')
  releaseConfig.push('is_official_build=true')

// Generate linker map for Linux.
if (targetOs == 'linux')
  releaseConfig.push('generate_linker_map=true')

gen('out/Component', componentConfig)
gen('out/Debug', debugConfig)
gen('out/Release', releaseConfig)

function gen(dir, args) {
  spawnSync('gn', ['gen', dir, `--args=${commonConfig.concat(args).join(' ')}`])
}
