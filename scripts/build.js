#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {argv, execSync} = require('./common')

const dir = argv.length > 0 ? argv[0] : 'out/Debug'

execSync(`ninja -C ${dir}`)
