/*
** $Id: op_cmc.c,v 1.3 2007-05-29 00:40:27 heroine Exp $
**
*/
#include <libasm.h>
#include <libasm-int.h>

/*
  <instruction func="op_cmc" opcode="0xf5"/>
*/

int op_cmc(asm_instr *new, u_char *opcode, u_int len, asm_processor *proc) 
{
  new->ptr_instr = opcode;
  new->len += 1;
  new->instr = ASM_CMC;
  return (new->len);
}
