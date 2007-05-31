/**
 * @file op_mov_eax_ref_iv.c
 * $Id: op_mov_eax_ref_iv.c,v 1.4 2007-05-29 00:40:27 heroine Exp $
 *
 */
#include <libasm.h>
#include <libasm-int.h>

/**
 * Instruction handler for opcode 0xa1
 *
  <instruction func="op_mov_eax_ref_iv" opcode="0xa1"/>
*/

int op_mov_eax_ref_iv(asm_instr *new, u_char *opcode, u_int len, 
		      asm_processor *proc) 
{
  new->instr = ASM_MOV;
  new->ptr_instr = opcode;
  new->len += 1;
  new->type = ASM_TYPE_ASSIGN;
#if LIBASM_USE_OPERAND_VECTOR
  new->len += asm_operand_fetch(&new->op1, opcode + 1, ASM_OTYPE_FIXED, new);
  new->op1.ptr = opcode;
  new->op1.type = ASM_OTYPE_FIXED;
  new->op1.regset = asm_proc_opsize(proc) ?
    ASM_REGSET_R16 : ASM_REGSET_R32;
  new->op1.base_reg = ASM_REG_EAX;
  new->len += asm_operand_fetch(&new->op2, opcode + 1, ASM_OTYPE_OFFSET, new);
#else
  new->op1.content = ASM_OP_BASE | ASM_OP_FIXED;
  new->op1.ptr = opcode;
  new->op1.type = ASM_OTYPE_FIXED;
  new->op1.regset = asm_proc_opsize(proc) ?
    ASM_REGSET_R16 : ASM_REGSET_R32;
  new->op1.base_reg = ASM_REG_EAX;
  
  new->op2.type = ASM_OTYPE_OFFSET;
  new->op2.content = ASM_OP_VALUE | ASM_OP_REFERENCE;
  new->op2.len = 4;
  new->op2.ptr = opcode + 1;
  
  memcpy(&new->op2.imm, opcode + 1, 4);
  new->len += 4;
#endif
  return (new->len);
}