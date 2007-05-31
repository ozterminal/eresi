/**
 * @file op_prefix_ds.c
 * $Id: op_prefix_ds.c,v 1.3 2007-05-29 00:40:27 heroine Exp $
 *
 * Changelog
 * 200-07-29 : instruction length was not incremented. fixed.
 */
#include <libasm.h>
#include <libasm-int.h>

/*
  <instruction func="op_prefix_ds" opcode="0x3e"/>
*/

int op_prefix_ds(asm_instr *new, u_char *opcode, u_int len, 
		 asm_processor *proc)
{
  if (!new->ptr_prefix)
    new->ptr_prefix = opcode;
  new->len += 1;
  new->prefix |= ASM_PREFIX_DS;
  return (proc->fetch(new, opcode + 1, len - 1, proc));
}