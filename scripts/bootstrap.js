#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {argv, clang, hostCpu, targetCpu, targetOs, execSync, spawnSync} = require('./common')
const {gnConfig, luaVersions} = require('./config')

// Get the arch of sysroot.
let sysrootArch = {
  x64: 'amd64',
  x86: 'i386',
  arm: 'armhf',
  arm64: 'arm64',
}[targetCpu]

if (process.platform == 'linux')
  execSync(`python3 building/tools/install-sysroot.py --arch ${sysrootArch}`)
else if (process.platform == 'win32')
  execSync('node scripts/download_nuget_packages.js')

execSync('git submodule sync --recursive')
execSync('git submodule update --init --recursive')
execSync('node scripts/download_gn.js')
execSync(`node scripts/download_node_headers.js node ${process.version} ${targetOs} ${targetCpu}`)
execSync(`node scripts/download_lua_sources.js lua ${luaVersions[0]}`)

if (clang && process.platform != 'darwin')
  execSync('python3 third_party/build-gn/tools/clang/scripts/update.py')

const commonConfig = gnConfig.slice()
if (process.env.CI === 'true')
  commonConfig.push('use_jumbo_build=true')
if (!(targetOs == 'win' && targetCpu == 'arm'))
  commonConfig.push(`node_version="${process.version}"`)
for (const arg of argv) {
  if (arg.startsWith('--extra-args='))
    commonConfig.push(...arg.substr(arg.indexOf('=') + 1).split(' '))
}

const componentConfig = [
  'is_component_build=true',
  'is_debug=true',
]
const debugConfig = [
  'is_component_build=false',
  'is_debug=true',
]
const releaseConfig = [
  'is_component_build=false',
  'is_debug=false',
  'is_official_build=true',
]

gen('out/Component', componentConfig)
gen('out/Debug', debugConfig)
gen('out/Release', releaseConfig)

function gen(dir, args) {
  spawnSync('gn', ['gen', dir, `--args=${commonConfig.concat(args).join(' ')}`])
}
