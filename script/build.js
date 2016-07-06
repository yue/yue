#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

const {ninja, execSync} = require('./common')

execSync(`${ninja} -C out/Default`)
