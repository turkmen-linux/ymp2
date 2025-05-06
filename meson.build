project(
  'ymp', 
  'c',
  version: '2.0.0'
)

add_project_arguments('-I'+meson.current_source_dir()+'/include', '-pthread',  language: 'c')
add_project_arguments('-fvisibility=hidden', '-Dvisible=__attribute__((visibility("default")))',  language: 'c')
add_project_arguments('-Wall', '-Wextra', '-Werror',  language: 'c')

srcs = []
deps = []

foreach src : run_command('find', 'src', '-type', 'f', '-iname', '*.c',check:true).stdout().strip().split('\n')
    srcs += src
endforeach

# Generate ctx.c file
ctx_c = ''
foreach ops : run_command('find', 'src/operations','-type','f',check:true).stdout().strip().split('\n')
    name = ops.split('/')[-1].split('.')[0]
    ctx_c = ctx_c + 'extern void '+name+'_init(OperationManager *manager);\n'
endforeach
ctx_c += '\nvoid ctx_init(OperationManager *manager){\n'
foreach ops : run_command('find', 'src/operations','-type','f',check:true).stdout().strip().split('\n')
    name = ops.split('/')[-1].split('.')[0]
    ctx_c += '  '+name+'_init(manager);\n'
endforeach
ctx_c += '}'


configure_file(
  input: 'src/ctx.c.in',
  output: 'ctx.c',
  configuration: {
    'VERSION': meson.project_version(),
    'DEBUG_MODE': get_option('debug') ? '1' : '0',
    'CTX': ctx_c
  }
)

srcs += [meson.current_build_dir()+'/ctx.c']


# Main library
libymp = library('ymp', srcs, dependencies: deps, link_args: ['-nostdlib', '-lc'])

subdir('docs')
subdir('examples')
