#include <stdio.h>
#include <stdlib.h>

#include <core/ymp.h>
#include <core/variable.h>

int main(int argc, char** argv){
    (void)argc, (void)argv;
    VariableManager *vars = variable_manager_new();
    variable_set_value(vars, "user", "pingu");
    char* user = variable_get_value(vars, "user");
    printf("%s\n", user);
    free(vars);
    Ymp *ymp = ymp_init();
    char* args_set[] = {"name", "pingu"};
    ymp_add(ymp, "set", (void*)args_set);
    char* args_get[] = {"name"};
    ymp_add(ymp, "get", (void*)args_get);
    ymp_run(ymp);
    return 0;
}