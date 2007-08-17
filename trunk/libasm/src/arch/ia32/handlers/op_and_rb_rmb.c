/*
** $Id: op_and_rb_rmb.c,v 1.5 2007-08-14 06:52:55 strauss Exp $
**
*/
#include <libasm.h>
#include <libasm-int.h>

/*
  <instruction func="op_and_rb_rmb" opcode="0x22"/>
*/

int op_and_rb_rmb(asm_instr *new, u_char *opcode, u_int len, 
                  asm_processor *proc) 
{
  new->len += 1;
  new->ptr_instr = opcode;
  new->instr = ASM_AND;

  new->type = ASM_TYPE_ARITH | ASM_TYPE_WRITEFLAG;
  new->flagswritten = ASM_FLAG_CF | ASM_FLAG_ZF | ASM_FLAG_PF |
                      ASM_FLAG_OF | ASM_FLAG_SF;

  new->len += asm_operand_fetch(&new->op1, opcode + 1, ASM_OTYPE_GENERALBYTE, 
                                new);
  new->len += asm_operand_fetch(&new->op2, opcode + 1, ASM_OTYPE_ENCODEDBYTE, 
                                new);

  return (new->len);
}