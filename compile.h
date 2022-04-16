#ifndef __SCC_COMPILE_H__
#define __SCC_COMPILE_H__

#include "vm.h"

// src code and dump
char *src, *src_dump;

int * symbol_ptr;

int token, token_val;
int line;

int ibp;

char* insts; // just for debug

int type; // pass type in recursive parse expr

// types of variables and functions in symbol table
enum {CHAR, INT, PTR};

// classes/keywords
enum {Num = 128, Fun, Sys, Glo, Loc, Id,
    Char, Int, Enum, If, Else, Return, Sizeof, While,
    // operators in precedence order
    Assign, Cond, Lor, Land, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge,
    Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak};

int load_src(char* file);

void keyword();

void parse();

void write_as();

#endif // __SCC_COMPILE_H__

