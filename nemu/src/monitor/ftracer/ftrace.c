#include "ftrace.h"
#include <isa.h>
#include <utils.h>

STACK_ENTRY header;
STACK_ENTRY *end = &header;

void init_ftracer(const char* elf_file){
  #ifdef CONFIG_FTRACE
  if(elf_file==NULL)
  return;
  
    init_elf(elf_file);
    // init header
    header.des_info = NULL;
    header.cur_info = NULL;
    header.next = NULL;
    header.addr = 0;
  #endif
}

#ifdef CONFIG_FTRACE
static void append(paddr_t cur, paddr_t des, int type){
    STACK_ENTRY *node = malloc(sizeof(STACK_ENTRY));
    end->next = node;
    node->next = NULL;
    node->addr = cur;
    node->cur_info = get_func_info(cur);
    node->des_info = get_func_info(des);
    node->type = type;

    end = node;
}
#endif

void stack_call(paddr_t cur, paddr_t des){
  #ifdef CONFIG_FTRACE
    append(cur, des, FT_CALL);
  #endif
}

void stack_return(paddr_t cur, paddr_t des){
  #ifdef CONFIG_FTRACE
    append(cur, des, FT_RET);
  #endif
}

#ifdef CONFIG_FTRACE_COND
    static char *action_name[] = {"Call", "Ret"};
#endif

void print_stack_trace(){
  #ifdef CONFIG_FTRACE_COND
    printf("====== " ASNI_FMT("Call Stack", ASNI_FG_BLUE) " ======\n");
    for (STACK_ENTRY* cur = &header; cur != end; cur = cur->next){
        STACK_ENTRY* r = cur->next;
        printf("<%#x>" ASNI_FMT(" %-12s ", ASNI_FG_BLUE) ASNI_FMT("%s", ASNI_FG_WHITE)  
            ASNI_FMT("\t<%#x> ", ASNI_FG_YELLOW) ASNI_FMT("%-12s \n", ASNI_FG_BLUE),  
            r->addr, 
            r->cur_info ? r->cur_info->fname : "", 
            action_name[r->type], 
            r->des_info ? r->des_info->faddr : 0, 
            r->des_info ? r->des_info->fname : "");
    }
  #endif
}

void error_finfo(){
  #ifdef CONFIG_FTRACE
  vaddr_t pc = cpu.pc;
  FUNC_INFO *info = get_func_info(pc);
  printf(ASNI_FMT("Some error happended at %s(%#x).\n", ASNI_FG_RED), info->fname, pc);
  #endif
}