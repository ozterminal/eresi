/*
** $Id: op_cwtl.c,v 1.3 2007-05-29 00:40:27 heroine Exp $
**
*/
#include <libasm.h>
#include <libasm-int.h>

/*
  <instruction func="op_cwtl" opcode="0x98"/>
*/

int     op_cwtl(asm_instr *new, u_char *opcode, u_int len, asm_processor *proc) {
  new->len += 1;
  new->ptr_instr = opcode;
  if (asm_proc_opsize(proc))
    new->instr = ASM_CBTW;
  else
    new->instr = ASM_CWTL;
  return (new->len);
}