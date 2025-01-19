const {ccWrapper, clang, targetCpu, targetOs} = require('./common')

// The version of nuget WebView2 package.
const webview2Version = '1.0.2903.40'

// The versions of language bindings to test against.
const nodeVersions = [
  '18.19.1',
  '20.18.1',
  '22.12.0',
]
const electronVersions = [
  '32.2.8',
  '33.3.1',
  '34.0.0',
]
const luaVersions = [
  '5.1.5',
  '5.3.6',
  '5.4.7',
]

// The version of gn.
const gnVersion = '0.11.3'

// The common build configurations.
const gnConfig = [
  `is_clang=${clang}`,
  `target_cpu="${targetCpu}"`,
  `lua_version="v${luaVersions[0]}"`,
  'fatal_linker_warnings=false',
]
if (ccWrapper) {
  gnConfig.push(`cc_wrapper="${ccWrapper}"`,
                'clang_use_chrome_plugins=false')
}
if (targetOs == 'win') {
  gnConfig.push(`webview2_version="${webview2Version}"`)
}

// The build configuration for sysroot.
const gnSysrootConfig = [
  'use_sysroot=true',
  'target_sysroot_dir="//third_party/sysroots"',
]
if (process.platform == 'linux') {
  gnConfig.push(...gnSysrootConfig);
}

module.exports = {
  webview2Version,
  nodeVersions,
  electronVersions,
  luaVersions,
  gnVersion,
  gnConfig,
}
