libdroidhal_sources = [
  'hal-implementation.c',
  'hal-service.c',
  'utils.c',
]

libdroidhal_deps = [
  dependency('gio-2.0'),
  dependency('libgbinder'),
]

libdroidhal_lib = shared_library('droidhal',
  libdroidhal_sources,
  dependencies: libdroidhal_deps,
  link_args: ['-Wl,--version-script,' + meson.current_source_dir() / 'libdroidhal.syms'],
  install: true,
)