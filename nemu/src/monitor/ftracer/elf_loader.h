#ifndef _ELF_LOADER_H_
#define _ELF_LOADER_H_

#include <common.h>
typedef struct
{
    char fname[64];
    paddr_t faddr;
    size_t fsize;
}FUNC_INFO;

#define FUNC_LIST_MAX 256

void init_elf(const char *elf_file);
FUNC_INFO* get_func_info(paddr_t addr);

#endif