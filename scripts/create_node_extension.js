#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, argv, targetCpu, execSync, spawnSync} = require('./common')

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
  // Set node nodever and runtime.
  `node_version="${nodever}"`,
  `node_runtime="${runtime}"`,
]

// Build the node module.
execSync(`node ./scripts/download_node_headers.js ${runtime} ${nodever}`)
spawnSync('gn', ['gen', 'out/Node', `--args=${args.join(' ')}`])
execSync('ninja -C out/Node node_yue')

// Generate a name conforming node's convention.
let arch = targetCpu
if (arch == 'x86')
  arch = 'ia32'
let shortver = nodever.substr(0, nodever.indexOf('.'))
const zipname = `node_yue_${version}_${runtime}_${shortver}_${process.platform}_${arch}`

// Create zip archive of the node module.
let zip = new JSZip()
zip.file('gui.node', fs.readFileSync('out/Node/gui.node'))
zip.generateNodeStream({streamFiles:true})
   .pipe(fs.createWriteStream(`out/Node/${zipname}.zip`))
