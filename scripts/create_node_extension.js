#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, argv, targetCpu, targetOs, mkdir, execSync, spawnSync} = require('./common')

const fs = require('fs')
const JSZip = require('./libs/jszip')

if (argv.length != 2) {
  console.error('Usage: create_node_extension runtime nodever')
  process.exit(1)
}

const runtime = argv[0]
const nodever = argv[1].startsWith('v') ? argv[1] : `v${argv[1]}`

const args = [
  // Same with Release build.
  'is_component_build=false',
  'is_debug=false',
  'is_official_build=true',
  'use_sysroot=true',
  `target_cpu="${targetCpu}"`,
  // Set node nodever and runtime.
  `node_version="${nodever}"`,
  `node_runtime="${runtime}"`,
]

if (targetOs == 'mac') {
  args.push('mac_deployment_target="10.9.0"',
            'mac_sdk_min="10.12"',
            'use_xcode_clang=true')
}

if (targetOs == 'linux') {
  args.push('is_clang=true',
            'clang_update_script="//building/tools/update-clang.py"')
  args.push('target_sysroot_dir="//third_party/"')
}

console.log(`Creating native extension for ${runtime} ${nodever} ${targetCpu}...`)

// Build the node module.
execSync(`node ./scripts/download_node_headers.js ${runtime} ${nodever} ${targetCpu}`)
spawnSync('gn', ['gen', 'out/Node', `--args=${args.join(' ')}`])
execSync('ninja -C out/Node node_yue')

// Generate a name conforming node's convention.
let shortver = nodever.substring(1, nodever.lastIndexOf('.'))
if (runtime == "node")
  shortver = shortver.substring(0, shortver.lastIndexOf('.'))
const zipname = `node_yue_${runtime}_${shortver}_${version}_${targetOs}_${targetCpu}`

// Strip the binaries on Linux.
if (targetOs == 'linux') {
  let strip = 'strip'
  if (targetCpu == 'arm')
    strip = 'arm-linux-gnueabihf-strip'
  else if (targetCpu == 'arm64')
    strip = 'aarch64-linux-gnu-strip'
  execSync(`${strip} out/Node/gui.node`)
}

// Create zip archive of the node module.
mkdir('out/Dist')
let zip = new JSZip()
zip.file('gui.node', fs.readFileSync('out/Node/gui.node'))
zip.generateNodeStream({
  streamFiles:true,
  compression: 'DEFLATE',
  compressionOptions: {
    level: 9
  }
}).pipe(fs.createWriteStream(`out/Dist/${zipname}.zip`))
