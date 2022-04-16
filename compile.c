#include "compile.h"


int load_src(char* file) {
    int fd, cnt;
    if ((fd = open(file, 0)) < 0) {
        printf("could not open source code(%s)\n", file);
        return -1;
    }
    if (!(src = src_dump = malloc(MAX_SIZE))) {
        printf("could not malloc(%lld) for source code\n", MAX_SIZE);
        return -1;
    }
    if ((cnt = read(fd, src, MAX_SIZE - 1)) <= 0) {
        printf("could not read source code(%lld)\n", cnt);
        return -1;
    }
    src[cnt] = 0;
    close(fd);
    return 0;
}

void tokenize() {
    char* ch_ptr;
    while(token = *src++) {
        if(token == '\n') {
            line++;
        } else if (token == '#') { // skip macro
            while (*src != 0 && *src != '\n') {
                src++;
            }
        } else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {
            ch_ptr = src - 1;
            while((*src >= 'a' && *src <= 'z') || (token >= 'A' && token <= 'Z') 
                || (*src >= '0' && *src <= '9') || (*src == '_')) {
                // use token store hash value
                token = token * 147 + *src++; 
            }
            
            // keep hash
            token = (token << 6) + (src - ch_ptr);
            symbol_ptr = symbol_table;
            
            // search same symbol in table
            while(symbol_ptr[Token]) {
                if (token == symbol_ptr[Hash] && !memcmp((char*)symbol_ptr[Name], ch_ptr, src - ch_ptr)) {
                    token = symbol_ptr[Token];
                    return;
                }
                symbol_ptr = symbol_ptr + SymSize;
            }
            // add new symbol
            symbol_ptr[Name] = (int)ch_ptr;
            symbol_ptr[Hash] = token;
            token = symbol_ptr[Token] = Id;
            return;
        } else if (token >= '0' && token <= '9') { // handle number
            // DEC, ch_ptr with 1 - 9
            if ((token_val = token - '0'))
                while (*src >= '0' && *src <= '9') token_val = token_val * 10 + *src++ - '0';
            //HEX, ch_ptr with 0x
            else if (*src == 'x' || *src == 'X')
                while ((token = *++src) && ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f')
                        || (token >= 'A' && token <= 'F')))
                    token_val = token_val * 16 + (token & 0xF) + (token >= 'A' ? 9 : 0);
            // OCT, start with 0
            else while (*src >= '0' && *src <= '7') token_val = token_val * 8 + *src++ - '0';
            token = Num;
            // printf("tokenize int: %d\n", token_val);
            return;
        } else if (token == '"' || token == '\'' ) { // handle string & char
            ch_ptr = data;
            while (*src != 0 && *src != token) {
                if((token_val = *src++) == '\\') {
                    // only support escape char '\n'
                    if ((token_val = *src++) == 'n') {
                        token_val = '\n';
                    }
                }
                 // store string to data segment
                if(token == '"') {
                    *data++ = token_val;
                }
            }
            src++;
            if (token == '"') {
                token_val = (int) ch_ptr;
            } else { // single char is Num
                token = Num;
            }
            return;
        } else  if (token == '/') {
            if(*src == '/') {
                // skip comment
                while(*src != 0 && *src != '\n') {
                    src++;
                }
            } else {
                //divide
                token = Div;
                return;
            } 
        } else if (token == '=') {
            if(*src == '=') {
                src++; 
                token = Eq;
            } else {
                token = Assign;
            }        
            return;
        } else if (token == '+') {
            if(*src == '+') {
                src++;
                token = Inc;
            } else {
                token = Add;
            }
            return;
        } else if (token == '-') {
            if(*src =='-') {
                src++;
                token = Dec;
            } else {
                token = Sub;
            }
            return;
        } else if (token == '!') {
            if(*src == '=') {
                src++;
                token = Ne;
            }
            return;
        } else if (token == '<') {
            if (*src == '=') {
                src++; 
                token = Le;
            } else if (*src == '<') {
                src++; 
                token = Shl;
            } else {
                token = Lt;
            } 
            return;
        } else if (token == '>') {
            if (*src == '=') {
                src++; 
                token = Ge;
            } else if (*src == '>') {
                src++;
                token = Shr;
            } else {
                token = Gt; 
            } 
            return;
        } else if (token == '|') {
            if (*src == '|') {
                src++; 
                token = Lor;
            } else {
                token = Or;
            } 
            return;
        } else if (token == '&') {
            if (*src == '&') {
                src++; 
                token = Land;
            } else { 
                token = And;
            } 
            return;
        } else if (token == '^') {
            token = Xor;
            return;
        } else if (token == '%') {
            token = Mod; 
            return;
        } else if (token == '*') {
            token = Mul; 
            return;
        } else if (token == '[') {
            token = Brak;
            return;
        } else if (token == '?') {
            token = Cond; 
            return;
        } else if (token == '~' || token == ';' 
            || token == '{' || token == '}' 
            || token == '(' || token == ')' 
            || token == ']' || token == ',' || token == ':') {
            return;
        }
    }
}

void keyword() {
    int i;
    src = "char int enum if else return sizeof while "
        "open read close printf malloc free memset memcmp exit void main";
    // add keywords to symbol table
    i = Char; while (i <= While) {
        tokenize(); 
        symbol_ptr[Token] = i++;
    }
    i = OPEN; while (i <= EXIT) {
        tokenize();
        symbol_ptr[Class] = Sys;
        symbol_ptr[Type] = INT;
        symbol_ptr[Value] = i++;
    }
    tokenize();
    symbol_ptr[Token] = Char; // handle void type;
    tokenize();
    main_ptr = symbol_ptr; // keep track of main
    src = src_dump;
}


void assert(int tk) {
    if (token != tk) {
        printf("line %ld: expect token: %dld(%c), get: %lld(%c)\n", line, tk, (char)tk, token, (char)token);
        exit(-1);
    }
    tokenize();
}

void check_local_id() {
    if(token != Id) {
        printf("line %lld: invalid identifer\n", line);
        exit(-1);
    }
    if(symbol_ptr[Class] == Loc) {
        printf("line %lld: duplicate declaration\n", line);
        exit(-1);
    }
}

void check_new_id() {
    if(token != Id) {
        printf("line %lld: invalid identifer\n", line);
        exit(-1);
    }
    if(symbol_ptr[Class]) {
        printf("line %lld: duplicate declaration\n", line);
        exit(-1);
    }
}

void parse_enum() {
    int i;
    i = 0; // enum index
    while (token != '}') {
        check_new_id();
        assert(Id);
        //handle custom enum index
        if(token == Assign) {
            assert(Assign);
            assert(Num);
            i = token_val;
        }
        symbol_ptr[Class] = Num;
        symbol_ptr[Type] = INT;
        symbol_ptr[Value] = i++;
        if(token == ",") {
            tokenize();
        }

    }
}

int parse_base_type() {
    // parse base type
    if(token == Char) {
        assert(Char);
        return CHAR;
    } else {
        assert(Int);
        return INT;
    }
}

void hide_global() {
    symbol_ptr[GClass] = symbol_ptr[Class];
    symbol_ptr[GType] = symbol_ptr[Type];
    symbol_ptr[GValue] = symbol_ptr[Value];
}

void recover_global() {
    symbol_ptr[Class] = symbol_ptr[GClass];
    symbol_ptr[Type] = symbol_ptr[GType];
    symbol_ptr[Value] = symbol_ptr[GValue];
}

void parse_expr(int precd) {
    int tmp_type, i;
    int* tmp_ptr;
    // const number
    if(token == Num) {
        tokenize();
        *++code = IMM;
        *++code = token_val;
        type = INT;
    } else if(token == '"') { // const string
        *++code = IMM;
        *++code = token_val; // string addr;
        assert('"'); 
        while(token == '"') { // handle multi-row
            assert('"');
        }
        data = (char*)((int)data + 8 & - 8); // add \0 for string & align 8
        type = PTR;
    } else if(token == Sizeof) {
        tokenize();
        assert('(');
        type = parse_base_type();
        while(token == Mul) {
            assert(Mul);
            type = type + PTR;
        }
        assert(')');
        *++code = IMM;
        *++code = (type == CHAR) ? 1 : 8;
        type = INT;
    } else if (token == Id) { // handle identifer: variable or function
        tokenize();
        tmp_ptr = symbol_ptr; // for recursive parse
        // function call
        if (token == '(') {
            assert('(');
            i = 0; // number of args
            while(token != ')') {
                parse_expr(Assign);
                *++code = PUSH; i++;
                if(token == ',') {
                    assert(',');
                }
            }
            assert(')');
            // native call
            if(tmp_ptr[Class] == Sys) {
                *++code = tmp_ptr[Value];
            } else if (tmp_ptr[Class] == Fun) { // func call
                *++code = CALL;
                *++code = tmp_ptr[Value];
            } else {
                printf("line %lld: invalid function call\n", line);
                exit(-1);
            }
            //delete stack frame for args
            if(i > 0) {
                *++code = DARG; 
                *++code = i;
            }
            type = tmp_ptr[Type];
        } else if(tmp_ptr[Class] == Num) { //handle enum value
            *++code = IMM;
            *++code = tmp_ptr[Value];
            type = INT;
        } else { //handle variables
            // local var, calculate addr base ibp
            if (tmp_ptr[Class] == Loc) {
                *++code = LEA;
                *++code = ibp - tmp_ptr[Value];
            } else if (tmp_ptr[Class] == Glo) { // global var
                *++code = IMM;
                *++code = tmp_ptr[Value];
            } else {
                printf("line %lld: invalid variable invalid class: %d\n", line, tmp_ptr[Class]);
                exit(-1);
            }
            type = tmp_ptr[Type];
            *++code = (type == CHAR) ? LC : LI;
        }
    } else  if (token == '(') { // cast or parenthesis
        assert('(');
        if (token == Char || token == Int) {
            tokenize();
            tmp_type = token - Char + CHAR;
            while (token == Mul) {
                assert(Mul);
                tmp_type = tmp_type + PTR;
            } 
            assert(')');
            parse_expr(Inc); // use precedence Inc represent all unary operators
            type = tmp_ptr;
        } else {
            parse_expr(Assign);
            assert(')');
        }
    } else if (token == Mul) { //derefer
        tokenize();
        parse_expr(Inc);
        if(type >= PTR) {
            type = type - PTR;
        } else {
            printf("line %lld: invalid dereference\n", line);
            exit(-1);
        }
        *++code = (type == CHAR) ? LC : LI;
    } else if (token == And) { // reference
        tokenize(); parse_expr(Inc);
        if (*code == LC || *code == LI) {
            code--; // rollback load by addr
        } else {
            printf("line %lld: invalid reference\n", line);
            exit(-1);
        }
        type = type + PTR;
    } else if (token == '!') { // Not
        tokenize(); parse_expr(Inc);
        *++code = PUSH; *++code = IMM; *++code = 0; *++code = EQ;
        type = INT;
    } else if(token == '~') { //bitwise
        tokenize(); parse_expr(Inc);
        *++code = PUSH; *++code = IMM; *++code = -1; *++code = XOR;
        type = INT;
    } else if(token == And) { // positive
        tokenize(); parse_expr(Inc); type = INT;
    } else if (token == Sub) { // negative
        tokenize(); parse_expr(Inc);
        *++code = PUSH; *++code = IMM; *++code = -1; *++code = MUL;
        type = INT;
    } else if (token == Inc || token == Dec) { // ++var --var
        i = token; tokenize(); parse_expr(Inc);
        // save var addr, then load var val
        if (*code == LC) {
            *code = PUSH; *++code = LC;
        } else if (*code == LI) {
            *code = PUSH; *++code = LI;
        } else {
            printf("line %lld: invalid Inc or Dec\n", line); 
            exit(-1);
        }
        *++code = PUSH; // save var val
        *++code = IMM; 
        *++code = (type > PTR) ? 8 : 1;
        *++code = (i == Inc) ? ADD : SUB; // calculate
        *++code = (type == CHAR) ? SC : SI; // write back to var addr
    } else {
        printf("line %lld: invalid expression\n", line); 
        exit(-1);
    }
    // use [precedence climbing] method to handle binary(or postfix) operators
    while (token >= precd) {
        tmp_type = type;
        // assignment
        if (token == Assign) {
            tokenize();
            if(*code == LC || *code == LI) {
                *code = PUSH;
            } else {
                printf("line %lld: invalid assignment\n", line);
                exit(-1);
            }
            parse_expr(Assign);
            type = tmp_type; // type can be case
            *++code = (type == CHAR) ? SC : SI;
        } else if (token == Cond) { // ? :, same as if stmt
            tokenize(); *++code = JZ; tmp_ptr = ++code;
            parse_expr(Assign); assert(':');
            *tmp_ptr = (int)(code + 3);
            *++code = JMP; tmp_ptr = ++code; //save endif addr
            parse_expr(Cond);
            *tmp_ptr = (int) (code + 1); // write back endif point
        }  else if (token == Lor) { // logic operators
            tokenize(); *++code = JNZ; tmp_ptr = ++code;
            parse_expr(Land); *tmp_ptr = (int)(code + 1); type = INT;}
        else if (token == Land) {
            tokenize(); *++code = JZ; tmp_ptr = ++code;
            parse_expr(Or); *tmp_ptr = (int)(code + 1); type = INT;
        } else if (token == Or)  {
            tokenize(); *++code = PUSH; parse_expr(Xor); *++code = OR;  type = INT;
        } else if (token == Xor) {
            tokenize(); *++code = PUSH; parse_expr(And); *++code = XOR; type = INT;
        } else if (token == And) {
            tokenize(); *++code = PUSH; parse_expr(Eq);  *++code = AND; type = INT;
        } else if (token == Eq)  {
            tokenize(); *++code = PUSH; parse_expr(Lt);  *++code = EQ;  type = INT;
        } else if (token == Ne)  {
            tokenize(); *++code = PUSH; parse_expr(Lt);  *++code = NE;  type = INT;
        } else if (token == Lt)  {
            tokenize(); *++code = PUSH; parse_expr(Shl); *++code = LT;  type = INT;
        } else if (token == Gt)  {
            tokenize(); *++code = PUSH; parse_expr(Shl); *++code = GT;  type = INT;
        } else if (token == Le)  {
            tokenize(); *++code = PUSH; parse_expr(Shl); *++code = LE;  type = INT;
        } else if (token == Ge)  {
            tokenize(); *++code = PUSH; parse_expr(Shl); *++code = GE;  type = INT;
        } else if (token == Shl) {
            tokenize(); *++code = PUSH; parse_expr(Add); *++code = SHL; type = INT;
        } else if (token == Shr) {
            tokenize(); *++code = PUSH; parse_expr(Add); *++code = SHR; type = INT;
        } else if (token == Add) { // arithmetic operators
            tokenize(); *++code = PUSH; parse_expr(Mul);
            // int pointer * 8
            if (tmp_type > PTR) {*++code = PUSH; *++code = IMM; *++code = 8; *++code = MUL;}
            *++code = ADD; type = tmp_type;
        } else if (token == Sub) {
            tokenize(); *++code = PUSH; parse_expr(Mul);
            if (tmp_type > PTR && tmp_type == type) {
                // pointer - pointer, ret / 8
                *++code = SUB; *++code = PUSH;
                *++code = IMM; *++code = 8;
                *++code = DIV; type = INT;}
            else if (tmp_type > PTR) {
                *++code = PUSH;
                *++code = IMM; *++code = 8;
                *++code = MUL;
                *++code = SUB; type = tmp_type;}
            else *++code = SUB;
        } else if (token == Mul) {
            tokenize(); *++code = PUSH; parse_expr(Inc); *++code = MUL; type = INT;
        } else if (token == Div) {
            tokenize(); *++code = PUSH; parse_expr(Inc); *++code = DIV; type = INT;
        } else if (token == Mod) {
            tokenize(); *++code = PUSH; parse_expr(Inc); *++code = MOD; type = INT;
        } else if (token == Inc || token == Dec) { // var++, var--
            if (*code == LC) {*code = PUSH; *++code = LC;} // save var addr
            else if (*code == LI) {*code = PUSH; *++code = LI;}
            else {printf("%lld: invlid operator=%lld\n", line, token); exit(-1);}
            *++code = PUSH; *++code = IMM; *++code = (type > PTR) ? 8 : 1;
            *++code = (token == Inc) ? ADD : SUB;
            *++code = (type == CHAR) ? SC : SI; // save value ++ or -- to addr
            *++code = PUSH; *++code = IMM; *++code = (type > PTR) ? 8 : 1;
            *++code = (token == Inc) ? SUB : ADD; // restore value before ++ or --
            tokenize();
        } else if (token == Brak) { // a[x] = *(a + x)
            assert(Brak); *++code = PUSH; parse_expr(Assign); assert(']');
            if (tmp_type > PTR) {*++code = PUSH; *++code = IMM; *++code = 8; *++code = MUL;}
            else if (tmp_type < PTR) {printf("line %lld: invalid index op\n", line); exit(-1);}
            *++code = ADD; type = tmp_type - PTR;
            *++code = (type == CHAR) ? LC : LI;
        } else {
            printf("%lld: invlid token=%lld\n", line, token); exit(-1);
        }
    }
}

void parse_stmt() {
    int *a;
    int *b;
    if(token == If) {
        assert(If);
        assert('(');
        parse_expr(Assign);
        assert(')');
        *++code = JZ; b = ++code; // JZ to false
        parse_stmt(); // parse true stmt
        if(token == Else) {
            assert(Else);
            *b = (int) (code + 3); // write back false point
            *++code = JMP; b = ++code; // JMP to endif
            parse_stmt(); // parse false stmt
        }
        *b = (int) (code + 1); // write back endif point
    } else if (token == While) {
        assert(While);
        a = code + 1;
        assert('('); // write loop point
        parse_expr(Assign);
        assert(')');
        *++code = JZ; b = ++code; // JZ to endloop
        parse_stmt();
        *++code = JMP; *++code = (int)a; // JMP to loop point
        *b = (int) (code + 1); // write back endloop point
    } else if(token == Return) {
        assert(Return);
        if (token != ';')
        parse_expr(Assign);
        assert(';');
        *++code = RET;
    } else if (token == '{') {
        assert('{');
        while(token != '}') {
            parse_stmt(Assign);
        }
        assert('}');
    } else if (token == ';') {
        assert(';');
    } else {
        parse_expr(Assign);
        assert(';');
    }
}

void parse_param() {
    int type, i;
    i = 0;
    while(token != ')') {
        type = parse_base_type();
        // parse pointer start
        while(token == Mul) {
            assert(Mul);
            type = type + PTR;
        }
        check_local_id();
        assert(Id);
        hide_global();
        symbol_ptr[Class] = Loc;
        symbol_ptr[Type] = type;
        symbol_ptr[Value] = i++;
        if(token == ',') {
            assert(',');
        }
    }
    ibp = ++i;    
}

void parse_func() {
    int type, i;
    i = ibp; // bp handle by NVAR itself
    // local variables must be declare in advance
    while(token == Char || token == Int) {
        type = parse_base_type();
        while(token != ';') {
            while(token == Mul) {
                assert(Mul);
                type = type + PTR;
            }
            check_local_id();
            assert(Id);
            hide_global();
            symbol_ptr[Class] = Loc;
            symbol_ptr[Type] = type;
            symbol_ptr[Value] = ++i;
            if(token == ',') {
                assert(',');
            }
        }
        assert(';');
    }
    // new stack frame for vars
    *++code = NVAR;
    *++code = i - ibp;
    while(token != '}') {
        parse_stmt();
    }
    if(*code != RET) {
        *++code = RET; //void function;
    }
    // recover global variables
    symbol_ptr = symbol_table;
    while(symbol_ptr[Token]) {
        if (symbol_ptr[Class] == Loc) {
            recover_global();
        }
        symbol_ptr = symbol_ptr + SymSize;
    }
}

void parse() {
    int type, base_type;
    int* p;
    line = 1; token = 1;
    while (token > 0) {
        tokenize();
        // parse enum
        if (token == Enum) {
            assert(Enum);
            if(token != '{'){
                assert(Id); // skip enum name
            }
            assert('{');
            parse_enum();
            assert('}');
        } else if (token == Int || token == Char) {
            base_type = parse_base_type();
            // parse var or func definition
            while(token != ';' && token != '}') {
                // parse pointer star
                type = base_type;
                while(token == Mul) {
                    assert(Mul);
                    type = type + PTR;
                }
                check_new_id();
                assert(Id);
                symbol_ptr[Type] = type;
                if(token == '(') {
                    // function
                    symbol_ptr[Class] = Fun;
                    symbol_ptr[Value] = (int)(code + 1);
                    assert('(');
                    parse_param();
                    assert(')');
                    assert('{');
                    parse_func();
                } else {
                    // variable
                    symbol_ptr[Class] = Glo;
                    symbol_ptr[Value] = (int) data;
                    data = data + 8;
                }
                //handle int a, b, c;
                if(token == ',') {
                    assert(',');
                }
            }
        }
    }
}

void write_as() {
    int fd;
    char* buffer;
    insts = "IMM ,LEA ,JMP ,JZ  ,JNZ ,CALL,NVAR,DARG,RET ,LI  ,LC  ,SI  ,SC  ,PUSH,"
        "OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
        "OPEN,READ,CLOS,PRTF,MALC,FREE,MSET,MCMP,EXIT,";
    fd = open("assemble", 0x0001 | 0x0200);
    buffer = malloc(100);
    while(code_dump < code) {
        sprintf(buffer, "(%lld) %8.4s", ++code_dump, insts + (*code_dump * 5));
        write(fd, buffer, strlen(buffer));
        if(*code_dump < RET) {
            sprintf(buffer, " %lld\n", *++code_dump);
        } else {
            buffer[0] = '\n'; buffer[1] = '\0';
        }
        write(fd, buffer, strlen(buffer));
    }
    close(fd);
}