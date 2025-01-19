const {ccWrapper, clang, targetCpu, targetOs} = require('./common')

// The version of nuget WebView2 package.
const webview2Version = '1.0.2420.47'

// The versions of language bindings to test against.
const nodeVersions = [
  '18.19.1',
  '20.11.1',
  '21.7.1',
]
const electronVersions = [
  '27.3.5',
  '28.2.6',
  '29.1.1',
]
const luaVersions = [
  '5.3.6',
  '5.4.3',
  '5.1.5',
]

// The version of gn.
const gnVersion = '0.11.0'

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
