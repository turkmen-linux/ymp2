# Test files
foreach src : run_command('find', '.', '-type', 'f', '-iname', '*.c',check:true).stdout().strip().split('\n')
    executable('example-'+src.split('/')[-1].split('.')[0], src, link_with: libymp)
endforeach

