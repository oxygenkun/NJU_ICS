#ifndef _FTRACER_H_
#define _FTRACER_H_

#include "elf_loader.h"

enum FUNC_T{
    FT_CALL,
    FT_RET,
} func_type;
// #define FT_CALL 0
// #define FT_CALL 1

typedef struct __STACK_ENTRY_{
    FUNC_INFO* cur_info;
    FUNC_INFO* des_info;
    paddr_t addr;
    int type;
    struct __STACK_ENTRY_ *next;
}STACK_ENTRY;

void init_ftracer(const char* elf_file);

void stack_return(paddr_t cur, paddr_t des);

void stack_call(paddr_t cur, paddr_t des);

void print_stack_trace();

void error_finfo();

#endif