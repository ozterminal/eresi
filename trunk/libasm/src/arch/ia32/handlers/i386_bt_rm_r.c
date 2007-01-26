/*
** $Id: i386_bt_rm_r.c,v 1.1 2007-01-26 14:18:37 heroine Exp $
**
*/
#include <libasm.h>
#include <libasm-int.h>

/*
  <i386 func="i386_bt_rm_r" opcode="0xa3"/>
*/

int i386_bt_rm_r(asm_instr *new, u_char *opcode, u_int len,
                 asm_processor *proc) {
  struct s_modrm        *modrm;
  
  modrm = (struct s_modrm *) opcode + 1;
  new->instr = ASM_BT;
  new->len += 1;
  new->op1.type = ASM_OTYPE_ENCODED;
  new->op2.type = ASM_OTYPE_GENERAL;
  operand_rmv_rv(new, opcode + 1, len - 1, proc);
    /*
    new->op1 = asm_create_operand(0, 0, 0, 0, proc);
    new->op1.content = OP_REG;
    new->op1.ptr = opcode + 1;
    new->op1.len = 1;
    new->op1.base_reg = modrm->m;
    
    
    new->op2 = asm_create_operand(0, 0, 0, 0, proc);
    new->op2.content = OP_REG;
    new->op2.ptr = opcode + 1;
    new->op2.len = 0;
    new->op2.base_reg = modrm->r;
    
    new->len += new->op1.len + new->op2.len;
    */
  return (new->len);
}
