project(
  'DiscImageCreator',
  'cpp',
  license : 'Apache-2.0',
  default_options : ['buildtype=release', 'prefix=/usr', 'cpp_std=c++11']
)

conf_data = configuration_data()

if target_machine.system().contains('linux') or target_machine.system().contains('darwin')
  build_date = run_command('date', '+%Y%m%d').stdout().strip()
  build_time = run_command('date', '+%H%M%S').stdout().strip()
endif

if target_machine.system().contains('linux')
  add_project_arguments('-include', 'defineForLinux.h', language : 'cpp')
endif

conf_data.set('build_date', build_date)
conf_data.set('build_time', build_time)

builddatetime_h = configure_file(
  input: 'DiscImageCreator/buildDateTime.h.in',
  output: 'buildDateTime.h',
  configuration: conf_data,
)

executable(
  meson.project_name(),
  sources: [
    builddatetime_h,
    'DiscImageCreator/DiscImageCreator.cpp',
    'DiscImageCreator/_external/NonStandardFunction.cpp',
    'DiscImageCreator/_external/abgx360.cpp',
    'DiscImageCreator/_external/aes.cpp',
    'DiscImageCreator/_external/aesni.cpp',
    'DiscImageCreator/_external/crc16ccitt.cpp',
    'DiscImageCreator/_external/crc32.cpp',
    'DiscImageCreator/_external/crc32ecma267.cpp',
    'DiscImageCreator/_external/crc6itu.cpp',
    'DiscImageCreator/_external/md5c.cpp',
    'DiscImageCreator/_external/platform_util.cpp',
    'DiscImageCreator/_external/prngcd.cpp',
    'DiscImageCreator/_external/rijndael-alg-fst.cpp',
    'DiscImageCreator/_external/sha1.cpp',
    'DiscImageCreator/_external/sha224-256.cpp',
    'DiscImageCreator/_external/sha384-512.cpp',
    'DiscImageCreator/_external/tinyxml2.cpp',
    'DiscImageCreator/_external/xxhash.cpp',
    'DiscImageCreator/_linux/defineForLinux.cpp',
    'DiscImageCreator/calcHash.cpp',
    'DiscImageCreator/check.cpp',
    'DiscImageCreator/convert.cpp',
    'DiscImageCreator/execIoctl.cpp',
    'DiscImageCreator/execScsiCmd.cpp',
    'DiscImageCreator/execScsiCmdforCD.cpp',
    'DiscImageCreator/execScsiCmdforCDCheck.cpp',
    'DiscImageCreator/execScsiCmdforDVD.cpp',
    'DiscImageCreator/execScsiCmdforFileSystem.cpp',
    'DiscImageCreator/execTapeCmd.cpp',
    'DiscImageCreator/fix.cpp',
    'DiscImageCreator/get.cpp',
    'DiscImageCreator/init.cpp',
    'DiscImageCreator/output.cpp',
    'DiscImageCreator/outputFileSystem.cpp',
    'DiscImageCreator/outputIoctlLog.cpp',
    'DiscImageCreator/outputScsiCmdLog.cpp',
    'DiscImageCreator/outputScsiCmdLogforCD.cpp',
    'DiscImageCreator/outputScsiCmdLogforDVD.cpp',
    'DiscImageCreator/set.cpp',
    'DiscImageCreator/xml.cpp',
  ],
  include_directories: 'DiscImageCreator/_linux',
  cpp_args: ['-Wno-unknown-pragmas'],
  install: true,
  install_dir: get_option('bindir'),
)

install_data(
  sources: [
    'Release_ANSI/default.dat',
    'Release_ANSI/driveOffset.txt',
  ],
  install_dir: get_option('datadir') / meson.project_name(),
)
