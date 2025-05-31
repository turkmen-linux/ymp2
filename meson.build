project(
  'ymp', 
  'c',
  version: run_command(
          ['bash', '-c', 'head -n 1 debian/changelog | sed "s/.*(//g;s/).*//g"']
          , check:true
      ).stdout().strip()
)

architecture = run_command('uname', '-m', check:true).stdout().strip()

add_project_arguments('-I'+meson.current_source_dir()+'/include', '-pthread',  language: 'c')
add_project_arguments('-fvisibility=hidden', '-Dvisible=__attribute__((visibility("default")))',  language: 'c')
add_project_arguments('-DPATH_MAX=1024', '-pthread', '-fPIC', language: 'c')
add_project_arguments('-Wno-unused-command-line-argument',  language: 'c') # for clang


if not get_option('debug')
    add_project_arguments('-DNDEBUG',  language: 'c')
else
    add_project_arguments('-Wall', '-Wextra', '-Werror',  language: 'c')
endif

srcs = []
deps = [
    dependency('libarchive', required:true),
    dependency('libcurl', required:true),
    dependency('openssl', required:true)
]


configure_file(
  input: 'include/config.h.in',
  output: 'config.h',
  configuration: {
    'VERSION': meson.project_version(),
    'BUILD_DIR': '/tmp/ymp-build/',
    'DISTRODIR': '/lib/turkman',
    'PLUGINDIR': get_option('plugindir'),
    'STORAGE': '/var/lib/ymp/',
  }
)

foreach src : run_command('find', 'src', '-type', 'f', '-iname', '*.c',check:true).stdout().strip().split('\n')
    srcs += src
endforeach

subdir('src')

start = [meson.current_source_dir()+'/src/start/'+architecture+'.s']
nostdlib = ['-nostdlib', '-lc']
if not get_option('startfiles')
    start = []
    nostdlib = []
endif


# Main library
libymp = library('ymp', srcs, dependencies: deps, c_args: ['-DLIBYMP', '-Dinvisible=extern'], link_args: nostdlib, install: true)

# Plugins
foreach src : run_command('find', 'plugin', '-type', 'f', '-iname', '*.c',check:true).stdout().strip().split('\n')
    name = src.replace('.c', '').split('/')[-1]
    library('ymp_'+name, [src], link_with: libymp, link_args: nostdlib, install: true)
endforeach


subdir('docs')
subdir('examples')

executable('ymp-cli', ['src/core/cli.c']+start, link_with: libymp, link_args: nostdlib, install: true)
