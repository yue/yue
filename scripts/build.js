#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {ninja, runSync} = require('./common')

process.exit(runSync(ninja, ['-C']).status)
