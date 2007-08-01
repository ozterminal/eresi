/*
**
** $Id: asm_sparc_fmovdr.c,v 1.9 2007-07-18 15:47:10 strauss Exp $
**
*/
#include "libasm.h"

int
asm_sparc_fmovdr(asm_instr * ins, u_char * buf, u_int len,
		 asm_processor * proc)
{
  struct s_decode_format3 opcode;
  struct s_asm_proc_sparc *inter;
  sparc_convert_format3(&opcode, buf);
  
  ins->type = ASM_TYPE_ASSIGN | ASM_TYPE_COMPARISON;

  inter = proc->internals;

  ins->instr = inter->fmovr_table[(((opcode.opf & 0x1f) - 6) * 8)
				  + opcode.rcond];

  ins->nb_op = 3;
  ins->op1.baser = ((opcode.rd & 1) << 5) | (opcode.rd & 0x1E);
  asm_sparc_op_fetch(&ins->op1, buf, ASM_SP_OTYPE_FREGISTER, ins);
  ins->op2.baser = ((opcode.rs2 & 1) << 5) | (opcode.rs2 & 0x1E);
  asm_sparc_op_fetch(&ins->op2, buf, ASM_SP_OTYPE_FREGISTER, ins);
  ins->op3.baser = opcode.rs1;
  asm_sparc_op_fetch(&ins->op3, buf, ASM_SP_OTYPE_REGISTER, ins);
  
  return 4;
}