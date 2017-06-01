#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {execSync} = require('./common')

execSync(`node ./scripts/bootstrap.js --target_cpu=${process.env.TARGET_CPU}`)
execSync(`node ./scripts/build.js -C out/${process.env.BUILD_TYPE}`)
