#include <stdbool.h>
#include <string.h>
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
    variables->priv_data = NULL;
    return variables;
}

void visible variable_set_value(VariableManager* variables, const char* name, const char* value){
    Variable* vars = (Variable*)variables->priv_data;
    for(size_t i=0; i< variables->length; i++){
        if(strcmp(name, vars[i].name) == 0){
            vars[i].value = (char*)value;
            return;
        }
    }
    if(variables->length >= variables->capacity){
        variables->capacity +=32;
        variables->priv_data = realloc(variables->priv_data, sizeof(VariableManager)*variables->capacity);
    }
    vars[variables->length] = (Variable)malloc(sizeof(Variable));
    vars[variables->length].name = (char*)name;
    vars[variables->length].value = (char*)value;
    variables->length++;
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
