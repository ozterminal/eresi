/*
**
** $Id: asm_sparc_done.c,v 1.4 2007-05-24 04:47:08 strauss Exp $
**
*/
#include "libasm.h"

int
asm_sparc_done(asm_instr * ins, u_char * buf, u_int len,
	       asm_processor * proc)
{
  struct s_decode_format3 opcode;
  struct s_asm_proc_sparc *inter;
  sparc_convert_format3(&opcode, buf, proc);

  inter = proc->internals;
  ins->instr = inter->op2_table[opcode.op3];
  
  ins->type = ASM_TYPE_RETPROC | ASM_TYPE_FLAG;
  ins->flags = ASM_SP_FLAG_C | ASM_SP_FLAG_V | ASM_SP_FLAG_Z | ASM_SP_FLAG_N;

  if (opcode.rd == 0)		/* DONE */
    ins->instr = ASM_SP_DONE;
  else if (opcode.rd == 1)
    ins->instr = ASM_SP_RETRY;	/* RETRY */
  else
    ins->instr = ASM_SP_BAD;
    
  return 4;

}
