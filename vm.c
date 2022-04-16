#include "vm.h"

int init_vm() {
    // allocate memory for virtual machine
    if (!(code = code_dump = malloc(MAX_SIZE))) {
        printf("could not malloc(%lld) for code segment\n", MAX_SIZE);
        return -1;
    }
    if (!(data = malloc(MAX_SIZE))) {
        printf("could not malloc(%lld) for data segment\n", MAX_SIZE);
        return -1;
    }
    if (!(stack = malloc(MAX_SIZE))) {
        printf("could not malloc(%lld) for stack segment\n", MAX_SIZE);
        return -1;
    }
    if (!(symbol_table = malloc(MAX_SIZE / 16))) {
        printf("could not malloc(%lld) for symbol table\n", MAX_SIZE/16);
        return -1;
    }
    memset(code, 0, MAX_SIZE);
    memset(data, 0, MAX_SIZE);
    memset(stack, 0, MAX_SIZE);
    memset(symbol_table, 0, MAX_SIZE / 16);
    return 0;
}

int run_vm(int argc, char** argv) {
    int op;
    int *tmp;
    // exit code for main
    bp = sp = (int*)((int)stack + MAX_SIZE);
    *--sp = EXIT;
    *--sp = PUSH; tmp = sp;
    *--sp = argc; *--sp = (int)argv;
    *--sp = (int)tmp;
    if(!(pc = (int*)main_ptr[Value])) {
        printf("main function is not defined\n");
        exit(-1);
    }
    cycle = 0;
    while(1) {
        cycle++; op = *pc++; //read instruction
        // load & save
        if (op == IMM) {
            ax = *pc++;  // load immediate(or global addr)
        } else if(op == LEA) {
            ax = (int)(bp + *pc++);  // load local addr
        } else if(op == LC) {
            ax = *(char*)ax;    // load char
        } else if(op == LI) {
            ax = *(int*)ax;     // load int
        } else if(op == SC) {
            *(char*) *sp++ = ax; // save char
        } else if(op == SI) {
            *(int*) *sp++ = ax;  // save int
        } else if(op == PUSH) {
            *--sp = ax;    // push ax to stack
        } else if(op == JMP) {
            pc = (int*) *pc;      // jump
        } else if(op == JZ) {
            pc = ax ? pc + 1 : (int*)*pc; // jump if ax == 0
        } else if(op == JNZ) {
            pc = ax ? (int*)*pc : pc + 1; // jump if ax != 0 
        } else if (op == OR) {      
            ax = *sp++ |  ax;
        } else if (op == XOR) {     
            ax = *sp++ ^  ax;
        } else if (op == AND) {    
            ax = *sp++ &  ax;
        } else if (op == EQ) {
            ax = *sp++ == ax;
        } else if (op == NE) {
            ax = *sp++ != ax;
        } else if (op == LT) {
            ax = *sp++ <  ax;
        } else if (op == LE) {
            ax = *sp++ <= ax;
        } else if (op == GT) {
            ax = *sp++ >  ax;
        } else if (op == GE) {
            ax = *sp++ >= ax;
        } else if (op == SHL) {
            ax = *sp++ << ax;
        } else if (op == SHR) {
            ax = *sp++ >> ax; 
        } else if (op == ADD) {
            ax = *sp++ +  ax;
        } else if (op == SUB) {
            ax = *sp++ -  ax;
        } else if (op == MUL) {
            ax = *sp++ *  ax;
        } else if (op == DIV) {
            ax = *sp++ /  ax;
        } else if (op == MOD) {
            ax = *sp++ %  ax;
        } else if (op == CALL) {
            // some complicate instructions for function call
            // call function: push pc + 1 to stack & pc jump to func addr(pc point to)
            *--sp = (int)(pc+1); 
            pc = (int*)*pc;
        } else if (op == NVAR) {
            // new stack frame for vars: save bp, bp -> caller stack, stack add frame
            *--sp = (int)bp; 
            bp = sp; 
            sp = sp - *pc++;
        } else if (op == DARG) {
            // delete stack frame for args: same as x86 : add esp, <size>
            sp = sp + *pc++;
        } else if (op == RET) {
            // return caller: retore stack, retore old bp, pc point to caller code addr(store by CALL) 
            sp = bp; 
            bp = (int*)*sp++; 
            pc = (int*)*sp++;
        }        
        // end for call function.
        // native call
        else if (op == OPEN)    {ax = open((char*)sp[1], sp[0]);}
        else if (op == CLOS)    {ax = close(*sp);}
        else if (op == READ)    {ax = read(sp[2], (char*)sp[1], *sp);}
        else if (op == PRTF)    {tmp = sp + pc[1] - 1; ax = printf((char*)tmp[0], tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5]);}
        else if (op == MALC)    {ax = (int)malloc(*sp);}
        else if (op == FREE)    {free((void*)*sp);}
        else if (op == MSET)    {ax = (int)memset((char*)sp[2], sp[1], *sp);}
        else if (op == MCMP)    {ax = memcmp((char*)sp[2], (char*)sp[1], *sp);}
        else if (op == EXIT)    {printf("exit(%lld)\n", *sp); return *sp;}
        else {
            printf("unkown instruction: %lld, cycle: %lld\n", op, cycle); return -1;
        }
    }
    return 0;
}
