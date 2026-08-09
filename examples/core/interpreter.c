#include <core/interpreter.h>
#include <core/ymp.h>

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    (void) ymp_init();
    return run_script(
        "print hello world\n"
        "set test 123\n"
        "if eq 1 3\n"
        "print test wrong\n"
        "endif\n"
        "if eq 1 1\n"
        "print test right\n"
        "endif\n");
}
