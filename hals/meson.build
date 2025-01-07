subdir('common')

systemd_units = [
  'libdroid-hal-lights.service',
]

executable(
  'libdroid-hal-lights',
  ['lights.c'],
  link_with: [libdroidhal_lib],
  dependencies: libdroidhal_deps + [dependency('gudev-1.0')],
  install: true
)


systemd = dependency('systemd')
if systemd.found()
  systemd_unit_dir = systemd.get_variable(pkgconfig: 'systemdsystemunitdir')

  foreach unit : systemd_units
    configure_file(
      input: unit + '.in',
      install: true,
      install_dir: systemd_unit_dir,
      output: unit,
      configuration: config_h,
    )
  endforeach
endif