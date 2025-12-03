# Ymp Plugin Development Guide

This document outlines the steps to develop a plugin using the **libymp** library.

## Prerequisites
Before compiling your source code with **libymp**, ensure that you have:

- The **libymp** library installed on your system.
- A compatible C compiler (e.g., **GCC**, **Clang**).
- The development headers for **libymp** available in your include path.

## Writing Your First Plugin

### Required Headers

Include the necessary headers in your source file:

```c
#include <core/ymp.h>
#include <core/operations.h>
```

### Plugin Initialization Function

Define the plugin's initialization function, which will be called when the plugin is loaded:

```c
visible void plugin_init(Ymp* ymp) {
    // Your initialization code here
}
```

### Defining a New Operation

To define a new operation, follow these steps:

1. Create an `Operation` structure.
2. Set the properties such as `name`, `alias`, `help`, and argument requirements.
3. Register the operation with the YMP manager.

Here’s an example of how to define a simple "hello" operation:

```c
#include <core/ymp.h>
#include <core/operations.h>

static int hello_fn(Ymp* ymp, const Argument* args, int arg_count) {
    // Implementation of the hello function
}

visible void plugin_init(Ymp* ymp) {
    Operation op;
    op.name = "hello";
    op.alias = "selam:hi:hola"; // Optional alias
    op.help = help_new(); // Optional help class
    help_add_parameter(op.help, "--arg", "Description"); // Argument description
    op.min_args = 0; // Minimum arguments required
    op.call = (callback) hello_fn; // Callback function

    operation_register(ymp->manager, op); // Register the operation
}
```

### Static Functions

Ensure that the `hello_fn` and any other callback functions you define are marked as `static` to restrict their visibility to this source file. This practice helps to avoid name collisions with other plugins.

### Compile and use Plugin
1. compile:
```bash
gcc -o libymp_hello.so hello.c $(pkg-config --cflags --libs libymp) -shared
```

2. copy libymp plugin
```bash
install libymp_hello.so $(ymp get plugindir)
```

3. use plugin
```bash
ymp hello 
```

### Complete Example

Here's a complete example of a simple YMP plugin:

```c
#include <stdio.h>

#include <core/ymp.h>
#include <core/operations.h>

static VariableManager* vars;

static int hello_fn(char** args) {
    printf("Hello, World!\n");
    printf("%s\n", variable_get_value(vars, "message"));
    return 0;
}

visible void plugin_init(Ymp* ymp) {
    vars = ymp->variables;
    Operation op;
    op.name = "hello";
    op.alias = "selam:hi:hola"
    op.help = help_new(); // Optional help class
    help_add_parameter(op.help, "--message", "Additional message");
    op.min_args = 0;
    op.call = (callback) hello_fn;

    operation_register(ymp->manager, op);
}
```



