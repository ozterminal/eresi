/**
 * \file libmjollnir/types.c
 *
 * Functions that use the typed instructions information in libasm
 *
 * Made by mayhem, thorkill, sk
 *
 * Updated Thu Dec 29 16:14:39 2006 mayhem
 */
#include "libmjollnir.h"



/**
 *  Depending on instruction type -based on IA32 instruction set-
 *
 *  ASM_TYPE_CONDBRANCH: jcc, loop, MAY NOT break execution flow
 *  ASM_TYPE_CALLPROC: calls break instruction flow but CAN restore it
 *  ASM_TYPE_IMPBRANCH, ASM_TYPE_RETPROC: jmp and ret break execution flow
 *
 *  The last two types reset g_prevaddr as execution flow won't be restored
 *  to following instruction.
 *
 * FIXME : this function must be tested on other architectures
 *
 */
int		mjr_asm_flow(mjrcontext_t *context)
{
  int		ilen;
  char		*md5;
  mjrfunc_t	*fun;
  asm_instr	*curins;
  elfsh_Addr	curvaddr;
  elfsh_Addr	dstaddr;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  curins   = &context->hist[MJR_HISTORY_CUR].instr;
  curvaddr = context->hist[MJR_HISTORY_CUR].vaddr;
  ilen     = asm_instr_len(curins);

  /* Switch on instruction types provided by libasm */
  switch (curins->type)
    {
    case ASM_TYPE_CONDBRANCH:
      dstaddr = mjr_insert_destaddr(context);

#if __DEBUG_FLOW__
      fprintf(D_DESC,
	      "[__DEBUG__] mjr_asm_flow: " XFMT " ASM_TYPE_CONDBRANCH T:" XFMT
	      " F:" XFMT"\n", curvaddr, dstaddr, curvaddr + ilen);
#endif
     if (dstaddr != -1)
       {
	 context->curblock->true  = dstaddr;
	 context->curblock->false = curvaddr + ilen;
	 context->curblock->type  = CALLER_JUMP;
	 context->curblock = 0;
       }
     break;
     
    case ASM_TYPE_CALLPROC:
      dstaddr = mjr_insert_destaddr(context);
      
#if __DEBUG_FLOW__
      fprintf(D_DESC,
	      "[__DEBUG__] mjr_asm_flow: " XFMT " ASM_TYPE_CALLPROC   T:" XFMT
	      " F:" XFMT" C:"XFMT"\n", curvaddr, dstaddr, curvaddr + ilen, (context->curfunc) ? context->curfunc->vaddr : 0x0);
#endif
      
      context->calls_seen++;

/* 20070102
 * FIXME: we should be able to resolve CALL 0x0 (dstaddr == 0), possible libasm bug or
 * mjollnir.
 *
 */
      if ((dstaddr) && (dstaddr != -1)){
        context->curblock->true  = dstaddr;
        context->curblock->false = curvaddr + ilen;
        context->curblock->type  = CALLER_CALL;

        if (context->curblock->true)
	    context->calls_found++;

        /* XXX: put this in a vector of fingerprinting techniques */
	fun = hash_get(&context->funchash, _vaddr2str(context->curblock->true));

	if (!fun)
	  fun = mjr_function_create(context, _vaddr2str(context->curblock->true),
				context->curblock->true, 0, 0);

        if (context->curfunc)
	    {
	      mjr_function_add_parent(fun, context->curfunc->vaddr,0);
	      mjr_function_add_child(context->curfunc, fun->vaddr, 0);
	    }

	// when a function start, we do a fingerprint of it
        md5 = mjr_fingerprint_function(context, context->curblock->true, 
				     MJR_FPRINT_TYPE_MD5);

        if (md5)
	  snprintf(fun->md5,sizeof(fun->md5),"%s",md5);

        context->curblock = 0;
        context->curfunc  = fun;
       }
      break;

    case ASM_TYPE_IMPBRANCH:
      dstaddr = mjr_insert_destaddr(context);

#if __DEBUG_FLOW__
      fprintf(D_DESC,
	      "[__DEBUG__] mjr_asm_flow: " XFMT " ASM_TYPE_IMPBRANCH  T:" XFMT 
	      " F: NULL \n", curvaddr, dstaddr);
#endif

      if (dstaddr != -1)
       {

        context->curblock->true  = dstaddr;
        context->curblock->false = 0;
        context->curblock->type  = CALLER_JUMP;
        context->curblock        = 0;
        context->hist[MJR_HISTORY_PREV].vaddr = 0;

       }
      break;

    case ASM_TYPE_RETPROC:

#if __DEBUG_FLOW__
      fprintf(D_DESC,"[__DEBUG__] mjr_asm_flow: " XFMT " ASM_TYPE_RETPROC\n", 
	      curvaddr);
#endif

      context->curblock->true  = 0;
      context->curblock->false = 0;
      context->curblock->type  = CALLER_RET;
      context->curblock        = 0;
      context->hist[MJR_HISTORY_PREV].vaddr = 0;
      break;

    default:
#if __DEBUG_FLOW__
	fprintf(D_DESC,"[__DEBUG__] mjr_asm_flow: %x DEFAULT %d\n",curvaddr, curins->type);
#endif
    break;

    }

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}


/**
 * Support function pointers computations
 *
 * Handle something like this : call *%reg
 *
 *  -> Check goto table and fixup destination
 *
 *  - start gdb, set breakpoint on a known pointer function, run the prog
 *  - check register value at breakpoint
 *  - start elfsh
 *  - addgoto <vaddr of call *%reg> <destaddr value of *%reg> 
 *
 * TODO:
 *  history of mov to reg when second operand was an address ;-)
 *
 *  804933c:       89 d1                   mov    %edx,%ecx
 *  804933e:       89 15 c0 e2 04 08       mov    %edx,0x804e2c0
 *  ...
 *  804936c:       ff d2                   call   *%edx
 *
 * FIXME : this function must be ported to the SPARC architecture
 */
elfsh_Addr		mjr_compute_fctptr(mjrcontext_t	*context)
{
  char		tmp[255];
  char		*ret;
  elfsh_Addr	dest;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);  

  printf(" [*] Found function pointer at %lx\n", 
	 (unsigned long) context->hist[MJR_HISTORY_CUR].vaddr);
  snprintf(tmp, sizeof(tmp), "%lx", 
	   (unsigned long) context->hist[MJR_HISTORY_CUR].vaddr);

  /* We deal with a constructed address */

  /* Specific pattern (simple one) */
  /* As we dont do any real dataflow analysis yet, we dont handle automatically
     complex case where addresses are  constructed in a complex way */

  if (context->hist[MJR_HISTORY_CUR].instr.instr   == ASM_CALL &&
      context->hist[MJR_HISTORY_PREV].instr.instr  == ASM_MOV  &&
      context->hist[MJR_HISTORY_PPREV].instr.instr == ASM_MOV)
    {
      dest = context->hist[MJR_HISTORY_PPREV].instr.op2.imm;

      if (dest < elfsh_get_entrypoint(context->obj->hdr))
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
			  "Invalid target vaddr for function pointer",
			  (elfsh_Addr) -1);

      /* Resolve target block */
      mjr_block_point(context, &context->hist[MJR_HISTORY_CUR].instr,
		      context->hist[MJR_HISTORY_CUR].vaddr, dest);

#if __DEBUG_MJOLLNIR__
      printf(" [*] 0x%lx Detected possible FUNCPTR at [%lx/%ld] \n",
	     (unsigned long) context->hist[MJR_HISTORY_CUR].vaddr,
	     (unsigned long) dest, (unsigned long) dest);
#endif

      PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, dest);
    }

  /* Instead we have a hash table that we can add element to using the 'addgoto'
     command of elfsh. That allow to tell which is the target address manually
     and make the graph continuous even with complex address construction */

  /* Happens when an address was manually inserted in the routing table */
  /* This allow to avoid the control flow graph to be broken if elfsh
     is not capable to recompute the target address */
  /* So the keys for this hash tables are the vaddr of the instruction that does
     the complex function pointer call */
  
 ret = (char *) hash_get(&goto_hash, tmp);
  if (!ret) 
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
		      "Unable to compute function pointer target",
		      (elfsh_Addr) -1);

  dest = strtol(ret, (char **) NULL, 16);
  if (dest)
    {

#if __DEBUG_MJOLLNIR__
      printf(" [*] Extended routing table found 0x%lx -> 0x%lx\n",
	     (unsigned long) context->hist[MJR_HISTORY_CUR].vaddr,
	     (unsigned long) dest);
#endif

      /* Resolve target block */
      mjr_block_point(context, &context->hist[MJR_HISTORY_CUR].instr,
		      context->hist[MJR_HISTORY_CUR].vaddr, dest);
    }

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, dest);
}





/**
 * This function add a new element to the linked list of callers 
 * of the current block.
 * 
 * It resolves operands of instruction which may modify the execution path.
 * 
 * If resolved to a virtual address, a new block is inserted.
 *
 * -> Return destination address inserted or -1 if unresolved
 *
 * FIXME: this function must be tested on the SPARC architecture
 *
 */
int		mjr_insert_destaddr(mjrcontext_t *context)
{
  int		ilen;
  elfsh_Addr	dest;
  asm_instr	*ins;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  dest = 0;
  ins  = &context->hist[MJR_HISTORY_CUR].instr;
  
  if (context->proc.type == ASM_PROC_IA32)
  {
  	/* The target block is called directly */
	if ((ins->op1.content & ASM_OP_VALUE) && !(ins->op1.content & ASM_OP_REFERENCE)) 
    {    
      	ilen    = asm_instr_len(ins);
      	asm_operand_get_immediate(ins, 1, 0, &dest);
      	dest   += ilen + context->hist[MJR_HISTORY_CUR].vaddr;
      	mjr_block_point(context, ins, context->hist[MJR_HISTORY_CUR].vaddr, dest);
    }

  	/* The target block is called indirectly : if we find a pattern that correspond 
       to an easy to predict function pointer, then we compute it */
 	else if (ins->op1.content & ASM_OP_BASE)
      dest = mjr_compute_fctptr(context);
  	else
      dest = -1;
  }
  else if (context->proc.type == ASM_PROC_SPARC)
  {
  	if (ins->instr == ASM_SP_CALL)
  	{
  	  if (ins->op1.type == ASM_SP_OTYPE_DISP30)
  	  {
  	  	dest = (ins->op1.imm * 4) + context->hist[MJR_HISTORY_CUR].vaddr;
  	  	mjr_block_point(context, ins, context->hist[MJR_HISTORY_CUR].vaddr, 
  	  					dest);
  	  	
  	  }
  	  else /* Indirect call (special case of JMPL) */
  	  	dest = -1;
  	}
  	else if (ins->instr == ASM_SP_JMPL) /* Indirect jump */
  	{
  	  dest = -1;
  	}
  	else if (ins->type == ASM_TYPE_CONDBRANCH)
  	{
  	  dest = (ins->op1.imm * 4) + context->hist[MJR_HISTORY_CUR].vaddr;
  	  mjr_block_point(context, ins, context->hist[MJR_HISTORY_CUR].vaddr, dest);
  	}
  }
  else
    dest = -1;
  
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, dest);
}






/* Old function of libmjollnir, maybe it does more than insert_destaddr, to make
   sure we can remove this */
/* Is not used anymore, could be removed, we keep it just to make sure because
   it seems like some cases are handled that are not yet in the new code */
/*
  int	mjr_get_calldest(mjrsession_t *sess, int *dest) 
  {
  int   ret = 0;
  
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  
  if (sess->cur->ihist[0].instr != ASM_CALL)
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,(ret));
  
  #if __DEBUG_CALLS__
  fprintf(D_DESC, "[__DEBUG_CALLS__] mjrGetCallDst: start - 0x%08x %d\n",
  sess->cur->curVaddr, sess->cur->ihist[0].op1.content);
  #endif
  
  if ((sess->cur->ihist[0].op1.content & ASM_OP_VALUE) &&
  !(sess->cur->ihist[0].op1.content & ASM_OP_REFERENCE))
  {
  if (asm_operand_get_immediate(&sess->cur->ihist[0], 1, 0, dest) == -1) 
  PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
  "Failed to get immediate operand", -1);
  ret = 1;
  }
  
  else if ((sess->cur->ihist[0].op1.content & ASM_OP_ADDRESS) &&
  sess->cur->ihist[1].instr == ASM_MOV &&
  sess->cur->ihist[2].instr == ASM_MOV &&
  !(sess->cur->ihist[2].op1.content & ASM_OP_SCALE) &&
  !(sess->cur->ihist[2].op2.content & ASM_OP_REFERENCE) &&
  sess->cur->ihist[2].op2.imm != 0x0) 
  {
  
  #if __DEBUG_OPERAND__
  asm_debug_operand(&sess->cur->ihist[0].op1);
  asm_debug_operand(&sess->cur->ihist[0].op2);
  asm_debug_operand(&sess->cur->ihist[2].op1);
  asm_debug_operand(&sess->cur->ihist[2].op2);
  #endif
  
  dest = (int *)sess->cur->ihist[2].op2.imm;
  
  #if __DEBUG_PTR__
  fprintf(D_DESC, "[__DEBUG_PTR__] mjrGetCallDst: PTR FUNCTION ! %x %x\n",
  sess->cur->curVaddr, (int)dest);
  #endif
  ret = 1;
  } 
  else
  {
  
  #if __DEBUG_OPERAND__
  fprintf(D_DESC, " [DM] Operand not supported! [vaddr = 0x%08x] %02X\n",
  sess->cur->curVaddr, sess->cur->ihist[0].op1.content);
  #endif
  
  PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
  "Operand content not supported.", -  #if __DEBUG_CALLS__
  fprintf(D_DESC, "[__DEBUG_CALLS__] mjrGetCallDst: vaddr: %x dest: %x\n",
  sess->cur->curVaddr,
  *dest);
  #endif
  
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,(ret));
  }
*/