#include <../include/rtl/rtl.h>
#include <cpu/exec.h>

def_EHelper(lui) { rtl_li(s, ddest, id_src1->imm); }

def_EHelper(auipc) { rtl_addi(s, ddest, &s->pc, id_src1->imm); }

def_EHelper(addi) { rtl_addi(s, ddest, dsrc1, id_src2->imm); }

def_EHelper(slti) { rtl_setrelopi(s, RELOP_LT, ddest, dsrc1, id_src2->imm); }

def_EHelper(sltiu) { rtl_setrelopi(s, RELOP_LTU, ddest, dsrc1, id_src2->imm); }

def_EHelper(slli) { rtl_slli(s, ddest, dsrc1, id_src2->imm & 0b111111u); }

def_EHelper(add) { rtl_add(s, ddest, dsrc1, dsrc2); }

def_EHelper(sub) { rtl_sub(s, ddest, dsrc1, dsrc2); }

def_EHelper(slt) { rtl_setrelop(s, RELOP_LT, ddest, dsrc1, dsrc2); }

def_EHelper(sltu) { rtl_setrelop(s, RELOP_LTU, ddest, dsrc1, dsrc2); }

def_EHelper(xor) { rtl_xor(s, ddest, dsrc1, dsrc2); }

def_EHelper(or) { rtl_or(s, ddest, dsrc1, dsrc2); }

def_EHelper(and) { rtl_and(s, ddest, dsrc1, dsrc2); }