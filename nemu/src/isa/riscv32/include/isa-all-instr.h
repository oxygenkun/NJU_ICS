#include <cpu/decode.h>
#include "../local-include/rtl.h"

#define INSTR_LIST(f) \
    f(lui) f(auipc) \
    f(jal) f(jalr)\
    f(beq) f(bne) f(blt) f(bltu) f(bge) f(bgeu) \
    f(addi) f(slti) f(sltiu) f(slli)\
    f(add) f(sub) f(slt) f(sltu) f(xor) f(or) f(and) f(sll) f(srl) f(sra)\
    f(lw) f(sw) \
    f(inv) f(nemu_trap)


def_all_EXEC_ID();
