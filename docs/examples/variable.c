#include <stdio.h>
#include <stdlib.h>

#include <variable.h>
int main(int argc, char** argv){
    (void)argc, (void)argv;
    VariableManager *vars = variable_manager_new();
    variable_set_value(vars, "user", "pingu");
    char* user = variable_get_value(vars, "user");
    printf("%s\n", user);
    free(vars);
    return 0;
}