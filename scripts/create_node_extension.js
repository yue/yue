#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {argv, execSync, spawnSync} = require('./common')

if (argv.length != 2) {
  console.error('Usage: create_node_extension runtime version')
  process.exit(1)
}

const runtime = argv[0]
const version = argv[1]

const args = [
  // Same with Release build.
  'is_component_build=false',
  'is_debug=false',
  'is_official_build=true',
  // Set node version.
  `node_version="${version}"`,
]

execSync(`node ./scripts/download_node_headers.js ${runtime} ${version}`)
spawnSync('gn', ['gen', 'out/Node', `--args=${args.join(' ')}`])
execSync('ninja -C out/Node node_yue')
