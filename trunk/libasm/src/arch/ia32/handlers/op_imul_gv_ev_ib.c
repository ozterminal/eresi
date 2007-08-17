/*
** $Id: op_imul_gv_ev_ib.c,v 1.5 2007-08-14 06:52:55 strauss Exp $
**
*/
#include <libasm.h>
#include <libasm-int.h>

/*
  <instruction func="op_imul_gv_ev_ib" opcode="0x6b"/>
*/

int     op_imul_gv_ev_ib(asm_instr *new, u_char *opcode, u_int len, 
			 asm_processor *proc) 
{
  int	olen;
  new->len += 1;
  new->ptr_instr = opcode;
  new->instr = ASM_IMUL;

  new->type = ASM_TYPE_ARITH | ASM_TYPE_WRITEFLAG;
  new->flagswritten = ASM_FLAG_OF | ASM_FLAG_CF;

  new->len += asm_operand_fetch(&new->op1, opcode + 1, ASM_OTYPE_GENERAL, new);
  new->len += (olen = asm_operand_fetch(&new->op2, opcode + 1, 
                                        ASM_OTYPE_ENCODED, new));
  new->len += asm_operand_fetch(&new->op3, opcode + 1 + olen, 
                                ASM_OTYPE_IMMEDIATEBYTE, new);

  return (new->len);
}