/*
**
** $Id: asm_sparc_popc.c,v 1.7 2007-07-06 21:18:08 strauss Exp $
**
*/
#include "libasm.h"

int
asm_sparc_popc(asm_instr * ins, u_char * buf, u_int len,
	       asm_processor * proc)
{
  struct s_decode_format3 opcode;
  struct s_asm_proc_sparc *inter;
  sparc_convert_format3(&opcode, buf);

  inter = proc->internals;
  ins->instr = inter->op2_table[opcode.op3];
  
  ins->type = ASM_TYPE_ASSIGN;

  if (opcode.rs1 == 0) {
    ins->instr = ASM_SP_POPC;
    ins->nb_op = 2;

    ins->op1.baser = opcode.rd;
    asm_sparc_op_fetch(&ins->op1, buf, ASM_SP_OTYPE_REGISTER, ins);

    if (opcode.i == 0) {
      ins->op2.baser = opcode.rs2;
      asm_sparc_op_fetch(&ins->op2, buf, ASM_SP_OTYPE_REGISTER, ins);
    }
    else {
      ins->op2.imm = opcode.imm;
      asm_sparc_op_fetch(&ins->op2, buf, ASM_SP_OTYPE_IMMEDIATE, ins);
    }
  }
  else
    ins->instr = ASM_SP_BAD;

  return 4;
}