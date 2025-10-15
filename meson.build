project(
  'ymp', 'c', 'vala',
  version: run_command(
          ['bash', '-c', 'head -n 1 debian/changelog | sed "s/.*(//g;s/).*//g"']
          , check:true
      ).stdout().strip()
)

architecture = run_command('uname', '-m', check:true).stdout().strip()

add_project_arguments('-Wno-unused-command-line-argument',  language: 'c') # for clang

if not get_option('debug')
    add_project_arguments('-DNDEBUG',  language: 'c')
else
    add_project_arguments('-Wall', '-Wextra', '-Werror',  language: 'c')
endif

srcs = []
l_args = []
lib_l_args = []
deps = [
    dependency('libarchive', required:true),
    dependency('libcurl', required:true),
    dependency('openssl', required:true)
]

start = [meson.current_source_dir()+'/src/start/'+architecture+'.s']
nostdlib = ['-nostdlib', '-lc']
if not get_option('startfiles')
    start = []
    nostdlib = []
endif

subdir('src')
subdir('data')

if get_option('plugins')
    add_project_arguments('-DPLUGIN_SUPPORT', language: 'c')
endif

l_args +=  ['-flto'] + nostdlib

if get_option('library')
    # Main library
    libymp = library('ymp', srcs, dependencies: deps, c_args: ['-Dinvisible=extern'], link_args: l_args + lib_l_args, install: true)
    executable('ymp-cli', ['src/core/cli.c']+start, c_args: ['-Dymp_main=main'], link_with: libymp, link_args: l_args, install: true)
else
    # single binary
    executable('ymp-cli', srcs+start, dependencies: deps, c_args: ['-Dymp_main=main', '-Dinvisible=extern'], link_args: l_args + lib_l_args, install: true)
endif
subdir('examples')
subdir('plugin')
subdir('docs')

