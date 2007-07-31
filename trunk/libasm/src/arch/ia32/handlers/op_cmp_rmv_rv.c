/*
** $Id: op_cmp_rmv_rv.c,v 1.5 2007-07-18 15:47:10 strauss Exp $
**
*/
#include <libasm.h>
#include <libasm-int.h>

/*
  <instruction func="op_cmp_rmv_rv" opcode="0x39"/>
*/

int op_cmp_rmv_rv(asm_instr *new, u_char *opcode, u_int len, 
		  asm_processor *proc)
{
  new->len += 1;
  new->type = ASM_TYPE_COMPARISON | ASM_TYPE_CONTROL;
  new->ptr_instr = opcode;
  new->instr = ASM_CMP;

  #if LIBASM_USE_OPERAND_VECTOR
  new->len += asm_operand_fetch(&new->op1, opcode + 1, ASM_OTYPE_ENCODED, 
				new);
  new->len += asm_operand_fetch(&new->op2, opcode + 1, ASM_OTYPE_GENERAL, 
				new);
  #else
  new->op1.type = ASM_OTYPE_ENCODED;
  new->op2.type = ASM_OTYPE_GENERAL;  
  operand_rmv_rv(new, opcode + 1, len - 1, proc);
  #endif
  return (new->len);
}