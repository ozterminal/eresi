/*
** $Id: op_shift_rmv_1.c,v 1.4 2007-08-15 21:30:20 strauss Exp $
**
*/
#include <libasm.h>
#include <libasm-int.h>

/*
  <instruction func="op_shift_rmv_1" opcode="0xd1"/>
 */

int op_shift_rmv_1(asm_instr *new, u_char *opcode, u_int len, 
                   asm_processor *proc)
{
  struct s_modrm        *modrm;

  modrm = (struct s_modrm *) opcode + 1;
  new->ptr_instr = opcode;
  new->len += 1;
  new->type = ASM_TYPE_ARITH | ASM_TYPE_WRITEFLAG;
  new->flagswritten = ASM_FLAG_PF | ASM_FLAG_ZF | ASM_FLAG_SF |
                        ASM_FLAG_CF | ASM_FLAG_OF;

  switch(modrm->r) {
  case ASM_REG_EDI:
    new->instr = ASM_SAR;
    break;
  case ASM_REG_ESP:
    new->instr = ASM_SHL;
    break;
  default:
    new->instr = ASM_SHR;
    break;
  }

  new->len += asm_operand_fetch(&new->op1, opcode + 1, ASM_OTYPE_ENCODED,
                                new);

  return (new->len);
}