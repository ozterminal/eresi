/**
 * @file asm_operand_fetch.c
 * $Id: asm_operand_fetch_fixed.c,v 1.2 2007-05-29 00:40:28 heroine Exp $
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

int     asm_operand_fetch_fixed(asm_operand *operand, u_char *opcode, 
				int otype, asm_instr *ins)
{
  operand->type = ASM_OTYPE_FIXED;
  return (0);
}
