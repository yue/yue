#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {argv, verbose, execSync} = require('./common')

let dir = 'out/Component'
const args = argv.filter((arg) => {
  if (arg.startsWith('out')) {
    dir = arg
    return false;
  } else {
    return true;
  }
})

execSync(`ninja ${verbose ? '-v' : ''} -C ${dir} ${args.join(' ')}`)
