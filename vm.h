#ifndef  __SCC_VM_H__
#define __SCC_VM_H__

#include <stdint.h>

#define int int64_t

int MAX_SIZE;

int * code,           // code segment
    * code_dump,      // code dump
    * stack;         // stack segment
char* data;          // data segment

// symbol table and pointer
int * symbol_table,
    * main_ptr;

int ax,              // common register
    cycle;

int * pc,            // pc register
    * sp,            // sp register
    * bp;            // bp register

// instruction set
enum {IMM, LEA, JMP, JZ, JNZ, CALL, NVAR, DARG, RET, LI, LC, SI, SC, PUSH,
    OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
    OPEN, READ, CLOS, PRTF, MALC, FREE, MSET, MCMP, EXIT};

// fileds of symbol table
enum {Token, Hash, Name, Class, Type, Value, GClass, GType, GValue, SymSize};   

int init_vm();

int run_vm(int argc, char** argv);

#endif // __SCC_VM_H__