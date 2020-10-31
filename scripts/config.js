const {clang, targetCpu, targetOs} = require('./common')

// The version of nuget WebView2 package.
const webview2Version = '1.0.622.22'

// The versions of Node.js to build against.
const nodeVersions = [
  '12.19.0',
  '14.15.0',
  '15.0.1',
]
const electronVersions = [
]

// The common build configurations.
const gnConfig = [
  `target_cpu="${targetCpu}"`,
  'use_allocator="none"',
  'use_allocator_shim=false',
  'use_partition_alloc=false',
  'fatal_linker_warnings=false',
]
if (clang) {
  gnConfig.push('is_clang=true',
              'clang_update_script="//building/tools/update-clang.py"')
} else {
  gnConfig.push('is_clang=false')
}
if (targetOs == 'mac') {
  gnConfig.push('mac_deployment_target="10.10.0"',
              'use_xcode_clang=true')
  if (targetCpu == 'arm64')
    gnConfig.push('mac_sdk_min="11.0"')
  else
    gnConfig.push('mac_sdk_min="10.15"')
} else if (targetOs == 'linux') {
  // Required for loading dynamic modules.
  gnConfig.push('use_cfi_icall=false')
} else if (targetOs == 'win') {
  gnConfig.push(`webview2_version="${webview2Version}"`)
}

// The build configuration for sysroot.
const gnSysrootConfig = [
  'use_sysroot=true',
  'target_sysroot_dir="//third_party/"',
  'debian_platform="stretch"'
]

module.exports = {
  webview2Version,
  nodeVersions,
  electronVersions,
  gnConfig,
  gnSysrootConfig,
}
