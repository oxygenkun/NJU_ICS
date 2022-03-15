#include <../include/rtl/rtl.h>
#include <cpu/exec.h>
#include <rtl/rtl.h>
#include <common.h>

def_EHelper(jal) {
  rtl_addi(s, s0, &s->pc, 4);
  rtl_addi(s, &s->dnpc, &s->pc, id_src1->imm);
  rtl_addi(s, ddest, s0, 0);
}

def_EHelper(jalr) {
  rtl_addi(s, s0, &s->pc, 4);
  rtl_addi(s, &s->dnpc, dsrc1, id_src2->imm);
  rtl_andi(s, &s->dnpc, &s->dnpc, ~1);
  rtl_addi(s, ddest, s0, 0);
}

def_EHelper(beq) {
  if((sword_t)*dsrc1 == (sword_t)*dsrc2){
    rtl_addi(s, &s->dnpc, &s->pc, id_dest->imm);
  }
}

def_EHelper(bne) {
  if((sword_t)*dsrc1 != (sword_t)*dsrc2){
    rtl_addi(s, &s->dnpc, &s->pc, id_dest->imm);
  }
}

def_EHelper(blt) {
  if((sword_t)*dsrc1 < (sword_t)*dsrc2){
    rtl_addi(s, &s->dnpc, &s->pc, id_dest->imm);
  }
}

def_EHelper(bltu) {
  if((word_t)*dsrc1 < (word_t)*dsrc2){
    rtl_addi(s, &s->dnpc, &s->pc, id_dest->imm);
  }
}

def_EHelper(bge) {
  if((sword_t)*dsrc1 >= (sword_t)*dsrc2){
    rtl_addi(s, &s->dnpc, &s->pc, id_dest->imm);
  }
}

def_EHelper(bgeu) {
  if((word_t)*dsrc1 >= (word_t)*dsrc2){
    rtl_addi(s, &s->dnpc, &s->pc, id_dest->imm);
  }
}