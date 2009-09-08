/**
 * @file asm_operand_fetch_immediate.c
ngroup ia-operands/g
 * @ingroup operands
 * $Id$
 */

#include <libasm.h>
#include <libasm-int.h>

/**
 *
 *
 */
/**
 * Decode data for operand type ASM_OTYPE_YDEST
ngroup ia-operands/g
 * @ingroup operands
 * @param operand Pointer to operand structure to fill.
 * @param opcode Pointer to operand data
 * @param otype
 * @param ins Pointer to instruction structure.
 * @return Operand length
 */

#if WIP
int     asm_operand_fetch_immediate(asm_operand *operand, u_char *opcode, int otype, 
				asm_instr *ins, int opt)
#else
int     asm_operand_fetch_immediate(asm_operand *operand, u_char *opcode, 
				    int otype, asm_instr *ins)
#endif
{
  operand->type = ASM_OTYPE_IMMEDIATE;
  operand->content = ASM_OP_VALUE;
  operand->ptr = opcode;
  operand->imm = 0;
  operand->len = asm_proc_opsize(ins->proc) ? 2 : 4;
  memcpy(&operand->imm, opcode, operand->len);
  return (operand->len);
}
