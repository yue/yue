#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, argv, targetCpu, targetOs, execSync, spawnSync} = require('./common')

const fs = require('fs')
const JSZip = require('./libs/jszip')

if (argv.length != 2) {
  console.error('Usage: create_node_extension runtime nodever')
  process.exit(1)
}

const runtime = argv[0]
const nodever = argv[1]

const args = [
  // Same with Release build.
  'is_component_build=false',
  'is_debug=false',
  'is_official_build=true',
  `target_cpu="${targetCpu}"`,
  // Set node nodever and runtime.
  `node_version="${nodever}"`,
  `node_runtime="${runtime}"`,
]

// Build the node module.
execSync(`node ./scripts/download_node_headers.js ${runtime} ${nodever}`)
spawnSync('gn', ['gen', 'out/Release', `--args=${args.join(' ')}`])
execSync('ninja -C out/Release node_yue')

// Generate a name conforming node's convention.
let shortver = nodever.substring(1, nodever.lastIndexOf('.'))
if (runtime == "node")
  shortver = shortver.substring(0, shortver.lastIndexOf('.'))
const zipname = `node_yue_${runtime}_${shortver}_${version}_${targetOs}_${targetCpu}`

// Strip the binaries on Linux.
if (targetOs == 'linux') {
  const strip = targetCpu.startsWith('arm') ? 'arm-linux-gnueabihf-strip' : 'strip'
  execSync(`${strip} out/Release/gui.node`)
}

// Create zip archive of the node module.
let zip = new JSZip()
zip.file('gui.node', fs.readFileSync('out/Release/gui.node'))
zip.generateNodeStream({streamFiles:true})
   .pipe(fs.createWriteStream(`out/Release/${zipname}.zip`))
