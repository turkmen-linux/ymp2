# Generate ctx.c file
ctx_c = ''
foreach ops : run_command('find', 'operations','-type','f',check:true).stdout().strip().split('\n')
    name = ops.split('/')[-1].split('.')[0]
    ctx_c = ctx_c + 'extern void '+name+'_init(OperationManager *manager);\n'
endforeach
ctx_c += '\nvoid ctx_init(OperationManager *manager){\n'
foreach ops : run_command('find', 'operations','-type','f',check:true).stdout().strip().split('\n')
    name = ops.split('/')[-1].split('.')[0]
    ctx_c += '  '+name+'_init(manager);\n'
endforeach
ctx_c += '}\n\n'
ctx_c += '#include <string.h>\n'
ctx_c += '__attribute__((section(".resource"))) char* resource(const char* name){\n'
foreach data : run_command('find', '../data','-type','f',check:true).stdout().strip().split('\n')
    data = data.substring(7)
    fdata = run_command('cat', '../data/'+data ,check:true).stdout()
    ctx_c += '    if(strcmp(name, ":'+data+'") == 0) {\n'
    ctx_c += '        return '
    foreach line : fdata.strip().split('\n')
        ctx_c += '\n          "'+line.replace('\\', '\\\\').replace('"', '\\"')+'\\n"'
    endforeach
    ctx_c += ';\n    }\n'
endforeach
ctx_c += '    return NULL;\n}\n\n'

configure_file(
  input: 'ctx.c.in',
  output: 'ctx.c',
  configuration: {
    'CTX': ctx_c
  }
)

srcs += [meson.current_build_dir()+'/ctx.c']

foreach src : run_command('find', '.', '-type', 'f', '-iname', '*.c',check:true).stdout().strip().split('\n')
    srcs += 'src/' + src
endforeach

if get_option('library')
    add_project_arguments('-fvisibility=hidden', '-Dvisible=__attribute__((visibility("default")))',  language: 'c')
else
    add_project_arguments('-fvisibility=hidden', '-Dvisible=', language: 'c')
endif

add_project_arguments('-I'+meson.current_source_dir()+'/../include',  language: 'c')
add_project_arguments('-DPATH_MAX=1024', '-pthread', '-fPIC', '-fno-plt', language: 'c')
