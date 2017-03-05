#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {argv, execSync} = require('./common')

if (argv.length != 2) {
  console.error('Usage: create_node_extension runtime version')
  process.exit(1)
}

const runtime = argv[0]
const version = argv[1]

execSync(`node ./scripts/download_node_headers.js ${runtime} ${version}`)
execSync('gn gen out/Debug', {env: {NODE_VERSION: version}})
execSync('gn gen out/Release', {env: {NODE_VERSION: version}})
execSync('ninja -C out/Release node_yue')
