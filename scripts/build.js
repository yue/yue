#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {execSync} = require('./common')

const dir = process.argv.length > 3 ? process.argv[2] : 'out/Debug'

execSync(`ninja -C ${dir}`)
