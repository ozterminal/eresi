/**
 * @file asm_operand_fetch.c
 * $Id: asm_operand_fetch_immediate.c,v 1.3 2007-05-29 00:40:28 heroine Exp $
 */

#include <libasm.h>
#include <libasm-int.h>

/**
 *
 *
 */
/**
 * Decode data for operand type ASM_OTYPE_YDEST
 * @param operand Pointer to operand structure to fill.
 * @param opcode Pointer to operand data
 * @param otype
 * @param ins Pointer to instruction structure.
 * @return Operand length
 */

int     asm_operand_fetch_immediate(asm_operand *operand, u_char *opcode, 
				    int otype, asm_instr *ins)
{
  operand->type = ASM_OTYPE_IMMEDIATE;
  operand->content = ASM_OP_VALUE;
  operand->ptr = opcode;
  operand->imm = 0;
  operand->len = asm_proc_opsize(ins->proc) ? 2 : 4;
  memcpy(&operand->imm, opcode, asm_proc_opsize(ins->proc) ? 2 : 4);
  return (operand->len);
}