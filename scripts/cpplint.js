#!/usr/bin/env node

// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {spawn} = require('child_process')
const {lstatSync, readdirSync} = require('fs')
const path = require('path')

require('./common')

// Do not lint following files.
const blacklist = [
  path.join('nativeui', 'gtk', 'util', 'x11_types.h'),
  path.join('nativeui', 'gtk', 'util', 'x11_util.cc'),
  path.join('nativeui', 'gtk', 'util', 'x11_util.h'),
  path.join('nativeui', 'mac', 'browser', 'nu_custom_protocol.h'),
  path.join('nativeui', 'mac', 'container_mac.h'),
  path.join('nativeui', 'mac', 'drag_drop', 'data_provider.h'),
  path.join('nativeui', 'mac', 'notification_center_mac.h'),
  path.join('nativeui', 'mac', 'nu_application_delegate.h'),
  path.join('nativeui', 'mac', 'nu_responder.h'),
  path.join('nativeui', 'mac', 'nu_table_cell.h'),
  path.join('nativeui', 'mac', 'nu_table_column.h'),
  path.join('nativeui', 'mac', 'nu_table_data_source.h'),
  path.join('nativeui', 'mac', 'nu_view.h'),
  path.join('nativeui', 'mac', 'nu_window.h'),
  path.join('nativeui', 'win', 'util', 'msg_util.h'),
  path.join('nativeui', 'util', 'aes.cc'),
  path.join('nativeui', 'util', 'aes.h'),
]

const sourceFiles = listFiles([
  'lua',
  'lua_yue',
  'nativeui',
  'node_yue',
  'v8binding'])
.filter((name) => {
  return name.endsWith('.cc') || name.endsWith('.h')
})
.filter((name) => {
  return !blacklist.includes(name)
})

// Call cpplint.
let hasError = false
const cpplint = path.join('building', 'tools', 'cpplint.py')
while (sourceFiles.length) {
  const chunck = sourceFiles.splice(0, 100)
  const child = spawn('python', [cpplint, ...chunck], {stdio: 'inherit'})
  child.on('close', (code) => {
    if (code != 0)
      hasError = true
  })
}

process.on('beforeExit', (code) => {
  process.exit(hasError ? 1 : 0)
})

// Util function to list files recursively
function listFiles(dirs) {
  let result = []
  for (const dir of dirs) {
    const subdirs = readdirSync(dir)
    for (const sub of subdirs) {
      const stat = lstatSync(path.join(dir, sub))
      if (stat.isFile())
        result.push(path.join(dir, sub))
      else if (stat.isDirectory())
        result = result.concat(listFiles([path.join(dir, sub)]))
    }
  }
  return result
}
