/*
** $Id: op_add_rv_rmv.c,v 1.5 2007-08-14 06:52:55 strauss Exp $
**
*/
#include <libasm.h>
#include <libasm-int.h>

/**
 * Handler for the add rv, rmv instruction, opcode 0x03
 * @param new Pointer to the instruction structure.
 * @param opcode Pointer to data to disassemble.
 * @param len Length of the data to disassemble.
 * @param proc Pointer to the processor structure.
 * @return Length of the disassembled instruction.
 */

int op_add_rv_rmv(asm_instr *new, u_char *opcode, u_int len, 
		  asm_processor *proc) 
{
  new->instr = ASM_ADD;
  new->len++;
  new->ptr_instr = opcode;

  new->type = ASM_TYPE_ARITH | ASM_TYPE_WRITEFLAG;
  new->flagswritten = ASM_FLAG_CF | ASM_FLAG_ZF | ASM_FLAG_PF |
                      ASM_FLAG_OF | ASM_FLAG_AF | ASM_FLAG_SF;

  new->len += asm_operand_fetch(&new->op1, opcode + 1, ASM_OTYPE_GENERAL, 
                                new);
  new->len += asm_operand_fetch(&new->op2, opcode + 1, ASM_OTYPE_ENCODED, 
                                new);

  return (new->len);
}