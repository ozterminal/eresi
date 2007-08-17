/*
** $Id: op_repnz.c,v 1.4 2007-08-15 21:30:20 strauss Exp $
**
*/
#include <libasm.h>
#include <libasm-int.h>

/*
  <instruction func="op_repnz" opcode="0xf2"/>
*/

int op_repnz(asm_instr *new, u_char *opcode, u_int len, asm_processor *proc) 
{
  if (!new->ptr_prefix)
    new->ptr_prefix = opcode;
  new->len += 1;
  new->prefix = ASM_PREFIX_REPNE;

  return (proc->fetch(new, opcode + 1, len - 1, proc));
}