#include <isa.h>
#include "local-include/reg.h"
#include <common.h>

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  for(int n=0; n < 32; n++){
    printf("%s\t\t" FMT_WORD "\t\t%d\n", reg_name(n, 0), gpr(n), gpr(n));
  }
  // TODO: print PC (real address)
  //printf("PC\t\t%04x\t\t%d\n", )
}

word_t isa_reg_str2val(const char *s, bool *success) {
  for(int i=0; i<32; ++i){
    if(strcmp(s,regs[i])==0){
      return gpr(i);
    }
  }
  success = false;
  return 0;
}
