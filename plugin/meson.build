# Plugins
if get_option('plugins') and get_option('library')
    foreach src : run_command('find', '.', '-type', 'f', '-iname', '*.c',check:true).stdout().strip().split('\n')
        name = src.replace('.c', '').split('/')[-1]
        library('ymp_'+name, [src], link_with: libymp, link_args: l_args, install_dir: get_option('plugindir'), install: true)
    endforeach
endif

