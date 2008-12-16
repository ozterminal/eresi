#include "libasm.h"

int     asm_arm_strbt(asm_instr * ins, u_char * buf, u_int len,
                   asm_processor * proc)
{
  LIBASM_PROFILE_FIN();

  ins->instr = ASM_ARM_STRBT;

  LIBASM_PROFILE_FOUT(4);
}
