#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <variable.h>

typedef struct {
    char* name;
    char* value;
    bool read_only;
} Variable;

visible VariableManager* global_variables;

visible VariableManager* variable_manager_new(){
    VariableManager *variables = (VariableManager*)malloc(sizeof(VariableManager));
    variables->capacity = 0;
    variables->length = 0;
    variables->priv_data = malloc(sizeof(Variable));
    return variables;
}

static void variable_set_value_fn(VariableManager* variables, const char* name, const char* value, bool read_only){
    Variable* vars = (Variable*)variables->priv_data;
    // Search for existing variable
    for(size_t i=0; i< variables->length; i++){
        if(!read_only && vars[i].read_only) {
            continue;
        }
        if(strcmp(name, vars[i].name) == 0){
            vars[i].value = (char*)value;
            return;
        }
    }
    if(variables->length >= variables->capacity){
        // Reallocate variable storage
        variables->capacity +=32;
        variables->priv_data = realloc(variables->priv_data, sizeof(VariableManager)*variables->capacity);
    }
    vars[variables->length].name = (char*)name;
    vars[variables->length].value = (char*)value;
    vars[variables->length].read_only = read_only;
    variables->length++;
    variables->priv_data = (void*)vars;
}

void visible variable_set_value(VariableManager* variables, const char* name, const char* value){
    variable_set_value_fn(variables, name, value, false);
}

void visible variable_set_value_read_only(VariableManager* variables, const char* name, const char* value){
    variable_set_value_fn(variables, name, value, true);
}


visible char* variable_get_value(VariableManager* variables, const char* name){
    Variable* vars = (Variable*)variables->priv_data;
    for(size_t i=0; i< variables->length; i++){
        if(strcmp(name, vars[i].name) == 0){
            return vars[i].value;
        }
    }
    return "";
}
