#include <cpu/decode.h>
#include "../local-include/rtl.h"

#define INSTR_LIST(f) \
    f(lui) f(auipc) \
    f(jal) f(jalr)\
    f(beq) f(bne) f(blt) f(bltu) f(bge) f(bgeu) \
    f(addi) f(slti) f(sltiu) f(xori) f(ori) f(andi) f(slli) f(srli) f(srai)\
    f(add) f(sub) f(slt) f(sltu) f(xor) f(or) f(and) f(sll) f(srl) f(sra)\
    f(mul) f(mulh) f(mulhu) f(mulhsu) f(div) f(divu) f(rem) f(remu) \
    f(lw) f(lh) f(lb) f(lhu) f(lbu) f(sw) f(sh) f(sb)\
    f(inv) f(nemu_trap)


def_all_EXEC_ID();
