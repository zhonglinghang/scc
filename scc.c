#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compile.h"


int32_t main(int32_t argc, char** argv) {
    MAX_SIZE = 128 * 1024 * 8; // 1MB
    // load source code
    if (load_src(*(argv+1)) != 0) return -1;
    // init virtual machine
    if (init_vm() != 0) return -1;
    // prepare keywords for symbol table
    keyword();
    // parse and generate vm instructions and save
    parse();
    // write assembles: vm instructions
    write_as();
    // run vm and execute instructions
    return run_vm(--argc, ++argv);
}