# Test files
foreach src : run_command('find', '.', '-type', 'f', '-iname', '*.c',check:true).stdout().strip().split('\n')
    executable('../test/'+src, src, link_with: libymp)
endforeach

