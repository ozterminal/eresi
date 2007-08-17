/*
** $Id: op_jno.c,v 1.4 2007-08-14 06:52:55 strauss Exp $
**
*/
#include <libasm.h>
#include <libasm-int.h>

/*
  <instruction func="op_jno" opcode="0x71"/>
*/


int  op_jno(asm_instr *new, u_char *opcode, u_int len, asm_processor *proc)
{
  new->len += 1;
  new->type = ASM_TYPE_CONDBRANCH;
  new->instr = ASM_BRANCH_NOT_OVERFLOW;
  new->ptr_instr = opcode;

  new->len += asm_operand_fetch(&new->op1, opcode + 1, ASM_OTYPE_JUMP, new);

  return (new->len);
}