#!/usr/bin/env node

// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

require('./common')

const fs     = require('fs')
const path   = require('path')
const marked = require('./libs/marked')
const yaml   = loadYaml()
const pug    = loadPug()

// Supported languages.
const langs = ['cpp', 'lua', 'js']

// Output dir.
const outputdir = path.join('out', 'Documents')

// Read docs and generate HTML pages.
const apis = fs.readdirSync('docs/api')
for (let api of apis) {
  if (!api.endsWith('.yaml')) continue
  const name = path.basename(api, '.yaml')
  const doc = yaml.load(fs.readFileSync(`docs/api/${api}`))
  for (let lang of langs) {
    const langDoc = pruneDocTree(lang, doc)
    const langdir = path.join(outputdir, lang, 'api')
    mkdir(langdir)
    fs.writeFileSync(path.join(langdir, `${name}.yaml`), yaml.dump(langDoc))
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

// Load pug.js from its browserify pack.
function loadPug() {
  const vm = require('vm')
  const script = new vm.Script(fs.readFileSync(`${__dirname}/libs/pug.js`))
  const sandbox = {}
  script.runInNewContext(sandbox)
  return sandbox.require('pug')
}

// Make dir and ignore error.
function mkdir(dir) {
  if (fs.existsSync(dir)) return
  mkdir(path.dirname(dir))
  fs.mkdirSync(dir)
}

// Parse |doc| tree and only keep nodes for |lang|.
function pruneDocTree(lang, doc) {
  doc = JSON.parse(JSON.stringify(doc))

  if (doc.lang_detail && doc.lang_detail[lang])
    doc.detail += '\n' + doc.lang_detail[lang]
  delete doc.lang_detail

  const categories = ['constructors', 'class_properties', 'class_methods',
                      'methods', 'events']
  for (let category of categories) {
    if (!doc[category]) continue
    let nodes = []
    for (let node of doc[category]) {
      if (!node.lang || (node.lang && node.lang.includes(lang)))
        nodes.push(pruneNode(lang, node))
    }
    if (nodes.length > 0)
      doc[category] = nodes
    else
      delete doc[category]
  }

  return doc
}

// Convert C++ representation to |lang| representation.
function pruneNode(lang, node) {
  node = JSON.parse(JSON.stringify(node))

  delete node.lang
  // Recursively prune the Dictionary type parameters.
  if (node.params) {
    for (let param in node.params) {
      let descriptor = node.params[param]
      if (descriptor.properties) {
        for (let property of descriptor.properties)
          pruneNode(lang, property)
      }
    }
  }

  if (node.signature) {
    node.id = generateIdForSignature(node.signature)
    node.signature = parseSignature(lang, node.signature)
  } else if (node.property) {
    node.id = parseArg('lua', node.property).name
    node.property = parseArg(lang, node.property)
  } else if (node.callback) {
    node.id = generateIdForSignature(node.callback)
    node.callback = parseSignature(lang, node.callback)
  }

  return node
}

// Parse the C++ signature string.
function parseSignature(lang, str) {
  let signature = {}
  let match = str.match(/^(\w+)\((.*)\)$/)
  if (match) {
    // Constructor type.
    signature.name = match[1]
    signature.args = match[2]
  } else {
    match = str.match(/^(.*) (\w+)\((.*)\).*$/)
    signature.returnType = parseType(lang, match[1])
    signature.name = match[2]
    signature.args = match[3]
  }
  signature.name = parseName(lang, signature.name)
  signature.args = parseArgs(lang, signature.args)
  return signature
}

// Parse args.
function parseArgs(lang, str) {
  if (str == '') return []
  let args = str.split(',').map(parseArg.bind(null, lang))
  return args
}

// Parse arg.
function parseArg(lang, str) {
  let match = str.trim().match(/(.+) (\w+)/)
  return { type: parseType(lang, match[1]), name: parseName(lang, match[2]) }
}

// Convert method name from C++ to |lang|.
function parseName(lang, str) {
  if (lang == 'cpp')
    return str
  else if (lang == 'lua')
    return str.replace(/_/g, '').toLowerCase()
  else if (lang == 'js')
    return (str[0].toLowerCase() + str.substr(1)).replace(/_([a-z])/g, (m, w) => {
      return w.toUpperCase()
    })
}

// Convert type name from C++ to |lang|.
function parseType(lang, str) {
  if (lang == 'cpp') return str
  // Strip C++ qualifiers.
  if (str.startsWith('const '))
    str = str.substr('const '.length)
  if (str.endsWith('*') || str.endsWith('&'))
    str = str.substr(0, str.length - 1)
  // Built-in types.
  if (lang == 'lua') {
    switch (str) {
      case 'bool': return 'boolean'
      case 'float': return 'number'
      case 'std::string': return 'string'
      case 'char': return 'string'
      case 'Dictionary': return 'table'
    }
  } else if (lang == 'js') {
    switch (str) {
      case 'bool': return 'Boolean'
      case 'float': return 'Number'
      case 'std::string': return 'String'
      case 'char': return 'String'
      case 'Dictionary': return 'Object'
    }
  }
  // Custom types usually have 1-to-1 maps.
  return str
}

// Generate a readable ID from signature string.
function generateIdForSignature(str) {
  // Convert to all lowercase names.
  let signature = parseSignature('lua', str)
  let id = signature.name
  for (let arg of signature.args)
    id += `-${arg.name}`
  return id
}
