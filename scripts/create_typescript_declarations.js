#!/usr/bin/env node

// Copyright 2020 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

const {version, execSync} = require('./common')
const {createZip} = require('./zip_utils')

const path = require('path')
const fs = require('fs-extra')

// Call create_docs on needed.
const docsDir = 'out/Dist/docs/js/api'
if (!fs.existsSync(docsDir))
  execSync('node ./scripts/create_docs.js')

// Generate declarations.
const target = 'out/Dist/index.d.ts'
fs.writeFileSync(target, generateDocs(docsDir))

// Write zip.
createZip()
  .addFile('out/Dist/index.d.ts', 'out/Dist')
  .writeToFile(`node_yue_types_${version}`)

// Scan |docsDir| and generate types declarations from JSON files.
function generateDocs(docsDir) {
  let output = 'declare module "gui" {\n'
  // Search and read json formated API docs.
  const files = fs.readdirSync(docsDir).filter(f => f.endsWith('.json'))
                                       .map(f => fs.readJsonSync(path.join(docsDir, f)))
  // Parse the docs.
  const modules = sortModules(files)
  for (const mod of modules) {
    const code = addModule(mod)
    if (!code) continue
    output += indent(code)
    output += '\n\n'
  }
  output += indent(addSignal())
  output += '}\n'
  return output
}

// Make sure base classes are always listed before inherited classes.
function sortModules(modules) {
  // Get inheritance information.
  const parents = []
  const children = {}
  const others = []
  for (const mod of modules) {
    if (mod.type != 'Class') {
      others.push(mod)
      continue
    }
    if (mod.inherit) {  // derived class
      const parentName = mod.inherit.name
      if (!children[parentName])
        children[parentName] = []
      children[parentName].push(mod)
    } else {  // base class
      parents.push(mod)
    }
  }
  // List inherited classes after their base classes.
  const result = []
  const addParentsAndChildren = (parents) => {
    for (const parent of parents) {
      result.push(parent)
      if (children[parent.name])
        addParentsAndChildren(children[parent.name])
    }
  }
  addParentsAndChildren(parents)
  // List others.
  others.sort((a, b) => {
    if (a.component != b.component)  // exported module comes first
      return a.component ? -1 : 1
    if (a.type != b.type)  // enum comes last
      return a.type == 'Object' ? -1 : 1
    return a.name.localeCompare(b.name)
  })
  result.push(...others)
  return result
}

function addModule(mod) {
  if (mod.name == 'Signal') {
    // Will add a generic separately.
    return null
  } else if (mod.type == 'Class') {
    return addClass(mod)
  } else if (mod.type == 'Object') {
    const hasStaticMembers = mod.class_methods || mod.class_properties
    return addClass(mod, !hasStaticMembers)
  } else if (mod.type == 'String' && mod.enums) {
    return addEnum(mod)
  } else {
    return null
  }
}

function addClass(mod, isInterface = false) {
  let output = `export ${isInterface ? 'interface' : 'class'} ${getModuleName(mod)} `
  if (mod.inherit)
    output += `extends ${getModuleName(mod.inherit)} `
  output += '{\n'
  let sub = ''
  if (!isInterface)
    sub += 'protected constructor();\n'
  if (mod.class_properties) {
    for (const property of mod.class_properties)
      sub += 'static ' + addProperty(property) + '\n'
  }
  if (mod.class_methods) {
    for (const method of mod.class_methods)
      sub += 'static ' + addMethod(method) + '\n'
  }
  if (mod.methods) {
    for (const method of mod.methods) {
      if (!method.abstract)
        sub += addMethod(method) + '\n'
    }
  }
  if (mod.properties) {
    for (const property of mod.properties)
      sub += addProperty(property) + '\n'
  }
  if (mod.delegates) {
    for (const delegate of mod.delegates)
      sub += addDelegate(delegate) + '\n'
  }
  if (mod.events) {
    for (const event of mod.events)
      sub += addEvent(event) + '\n'
  }
  output += indent(sub)
  output += '}'
  if (mod.singleton) {
    output += '\n\n'
    output += `const ${mod.name.toLowerCase()}: ${mod.name};`
  }
  return output
}

function addEnum(mod) {
  const values = mod.enums.map(e => e.name)
  return `export type ${getModuleName(mod)} = ${values.join(' | ')};`
}

function addProperty(property) {
  let name = property.name
  if (property.optional)
    name += '?'
  return name + ': ' + toTSType(property.type) + ';'
}

function addMethod(method) {
  return method.signature.name + toTSFunction(method.signature) + ';'
}

function addDelegate(delegate) {
  return delegate.signature.name + ': ' +  toTSFunction(delegate.signature, true) + ';'
}

function addEvent(event) {
  // TypeScript does not allow different types for get/set, so we have to use
  // any as type to allow event handler assignment.
  return event.signature.name + ': any;'
  // The ideal declaration should be:
  //   const func = toTSFunction(event.signature, true)
  //   const setType = `Signal<(${func})> | (${func}) | null`
  //   return `set ${event.signature.name}(handler: ${setType});\n` +
  //          `get ${event.signature.name}() : Signal<(${func})>`
}

// The declaration for signals.
function addSignal() {
  return 'export interface Signal<T> {\n' +
         '  connect(handler: T): number;\n' +
         '  disconnect(id: number): void;\n' +
         '  disconnectAll(): void;\n' +
         '}\n'
}

// Convert JavaScript type name to TypeScript type name.
function toTSTypeBase(type, isReturnType) {
  switch (type.name) {
    case 'Boolean':
      return 'boolean'
    case 'Number':
    case 'Integer':
      return 'number'
    case 'Accelerator':
    case 'KeyboardCode':
    case 'String':
      return 'string'
    case 'Object':
      return 'object'
    case 'Array':
      return (type.elementType ? toTSType(type.elementType) : 'any') + '[]'
    case 'Any':
    case 'Buffer':
      return 'any'
    case 'Color':
      return isReturnType ? 'number' : 'string | number'
    case 'std::tuple':
      return '[' + type.args.map(a => toTSType(a)).join(', ') + ']'
    default:
      return type.name.replace(/::/g, '')
  }
}

function toTSType(type, isReturnType = false) {
  const name = toTSTypeBase(type, isReturnType)
  return type.nullable ? name + ' | null' : name
}

// Convert to function signature.
function toTSFunction(sig, annoymous = false) {
  let params = ''
  if (sig.parameters)
    params = sig.parameters.map(p => p.name + ': ' + toTSType(p.type))
                           .join(', ')
  let returnType = 'void'
  if (sig.returnType)
    returnType = toTSType(sig.returnType, true)
  if (annoymous)
    return '(' + params + ') => ' + returnType
  else
    return '(' + params + '): ' + returnType
}

// Remove :: from module name.
function getModuleName(mod) {
  return mod.name.replace(/::/g, '')
}

// Add 2 spaces indention.
function indent(lines) {
  return lines.replace(/^(?!\s*$)/gm, ' '.repeat(2))
}
