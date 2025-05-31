cfg = {
    'VERSION': meson.project_version(),
    'BUILD_DIR': '/tmp/ymp-build/',
    'DISTRODIR': '/lib/turkman',
    'PLUGINDIR': get_option('plugindir'),
    'PREFIX': get_option('prefix'),
    'STORAGE': '/var/lib/ymp/',
  }
foreach src : run_command('find', '..', '-type', 'f', '-iname', '*.in',check:true).stdout().strip().split('\n')
    name = src.split('/')[-1]
    configure_file(
      input: src,
      output: name.replace('.in',''),
      configuration: cfg
)
endforeach
run_command(meson.get_compiler('c'), 'obj.s','-c','-o', meson.current_build_dir()/'empty.o',check:true)
srcs += [meson.current_source_dir()/'obj.s']
add_project_arguments('-I'+meson.current_build_dir(), '-pthread',  language: 'c')
# install data files
data = {
    meson.current_build_dir()+'/ymp.pc': get_option('libdir')/ 'pkgconfig',
    'application-x-ymp.svg'            : get_option('prefix')/ 'share/icons/hicolor/scalable/mimetypes',
    'ymp-mime.xml'                     : get_option('prefix') / 'share/mime/packages/',
    meson.current_build_dir() / 'empty.o': get_option('libdir')
}

foreach src, target : data
    install_data(src, install_dir : target)
endforeach
