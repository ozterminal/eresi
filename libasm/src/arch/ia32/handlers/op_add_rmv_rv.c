/*
** $Id: op_add_rmv_rv.c,v 1.4 2007-05-29 00:40:27 heroine Exp $
**
*/
#include <libasm.h>
#include <libasm-int.h>

/*
 * Opcode :             0x01
 * Instruction :                ADD
 * Operands:            Destination: encoded vector.
 * Source: is a vector register.
 */

int op_add_rmv_rv(asm_instr *new, u_char *opcode, u_int len,
                  asm_processor *proc)
{
  new->len++;
  new->type = ASM_TYPE_ARITH;
  new->ptr_instr = opcode;
  new->instr = ASM_ADD;
  #if LIBASM_USE_OPERAND_VECTOR
  new->len += asm_operand_fetch(&new->op1, opcode + 1, ASM_OTYPE_ENCODED, 
				new);
  new->len += asm_operand_fetch(&new->op2, opcode + 1, ASM_OTYPE_GENERAL, 
				new);
  #else
  new->op1.type = ASM_OTYPE_ENCODED;
  new->op2.size = ASM_OSIZE_VECTOR;
  new->op2.type = ASM_OTYPE_GENERAL;
  new->op2.size = ASM_OSIZE_VECTOR;
  operand_rmv_rv(new, opcode + 1, len - 1, proc);
#endif
  return (new->len);
}