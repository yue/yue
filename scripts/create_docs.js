#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

require('./common')

const fs     = require('fs')
const path   = require('path')
const marked = require('./libs/marked')
const yaml   = loadYaml()

// Supported languages.
const langs = ['cpp', 'js', 'lua']

// Output dir.
const outputdir = path.join('out', 'Documents')

// Read docs and generate HTML pages.
const apis = fs.readdirSync('docs/api')
for (let api of apis) {
  if (!api.endsWith('.yaml')) continue
  const name = path.basename(api, '.yaml')
  const doc = yaml.load(fs.readFileSync(`docs/api/${api}`))
  for (let lang of langs) {
    const html = docToHtml(doc, lang)
    const langdir = path.join(outputdir, lang, 'api')
    mkdir(langdir)
    fs.writeFileSync(path.join(langdir, `${name}.html`), html)
  }
}

// Load js-yaml from its browserify pack.
function loadYaml() {
  const vm = require('vm')
  const script = new vm.Script(fs.readFileSync(`${__dirname}/libs/yaml.js`))
  const sandbox = {}
  script.runInNewContext(sandbox)
  return sandbox.jsyaml
}

// Make dir and ignore error.
function mkdir(dir) {
  if (fs.existsSync(dir)) return
  mkdir(path.dirname(dir))
  fs.mkdirSync(dir)
}

// Parse doc tree and generate HTML document.
function docToHtml(doc, lang) {
  let html = ''
  html += `<h1>${doc.name}</h1>`
  html += `<div class="description">${marked(doc.description)}</div>`
  html += '<div class="detail">'
  if (doc.detail)
    html += marked(doc.detail)
  if (doc.lang_detail && doc.lang_detail[lang])
    html += marked(doc.lang_detail[lang])
  html += '</div>'
  if (doc.class_properties) {
    html += '<h2>Class Properties</h2>'
  }
  return html
}
