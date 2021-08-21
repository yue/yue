#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, clang, argv, targetCpu, targetOs, hostCpu, strip, execSync, spawnSync} = require('./common')
const {gnConfig, gnSysrootConfig} = require('./config')
const {createZip} = require('./zip_utils')

const fs = require('fs-extra')

if (argv.length != 2) {
  console.error('Usage: create_node_extension runtime nodever')
  process.exit(1)
}

const runtime = argv[0]
const nodever = argv[1].startsWith('v') ? argv[1] : `v${argv[1]}`

if (runtime == 'node' && targetOs == 'win' && targetCpu.startsWith('arm')) {
  console.error('Node.js does not have official binary for Windows on ARM')
  process.exit(1)
}

const args = gnConfig.concat([
  'use_jumbo_build=true',
  `node_version="${nodever}"`,
  `node_runtime="${runtime}"`,
  // Same with Release build.
  'is_component_build=false',
  'is_debug=false',
  'is_official_build=true',
])
if (targetOs == 'linux')
  args.push(...gnSysrootConfig)

console.log(`Creating native extension for ${runtime} ${nodever} ${targetCpu}...`)

// Build the node module.
execSync(`node ./scripts/download_node_headers.js ${runtime} ${nodever} ${targetOs} ${targetCpu}`)
spawnSync('gn', ['gen', 'out/Node', `--args=${args.join(' ')}`])
execSync('ninja -C out/Node node_yue')

// Strip the binaries on Linux.
if (targetOs == 'linux')
  strip('out/Node/gui.node')

// Run tests.
if (hostCpu == targetCpu)
  execSync(`node ./scripts/test_node_extension.js ${runtime} ${nodever}`)

// Generate a name conforming node's convention.
const shortver = nodever.substring(1, nodever.indexOf('.'))

// Create zip archive of the node module.
fs.ensureDirSync('out/Dist')
const zip = createZip({withLicense: true})
if (targetOs == 'win')
  zip.addFile('out/Node/WebView2Loader.dll', 'out/Node')
zip.addFile('out/Node/gui.node', 'out/Node')
   .writeToFile(`node_yue_${runtime}_${shortver}_${version}_${targetOs}_${targetCpu}`)
