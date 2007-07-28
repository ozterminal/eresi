/*
** delete.c for e2dbg
**    
** Delete breakpoints
**
** Started on  Tue Aug 16 09:38:03 2005 mayhem                                                                                                                   
**
** $Id: delete.c,v 1.4 2007-07-17 18:11:24 may Exp $
**
*/
#include "libe2dbg.h"






/* Delete a breakpoint */
int		cmd_delete()
{
  elfsh_Addr	addr;
  elfshbp_t	*bp;
  char		*name;
  int		prot;
  elfsh_SAddr	off;
  char		logbuf[BUFSIZ];

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  /* Lookup breakpoint */
  bp = e2dbg_breakpoint_lookup(revm_lookup_string(world.curjob->curcmd->param[0]));
  if (!bp)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Unknown breakpoint", -1);
  addr = bp->addr;
  
  /* Delete the breakpoint */
  prot = elfsh_munprotect(bp->obj, addr,  1);
  *(u_char *) addr = bp->savedinstr[0];
  elfsh_mprotect(addr, 1, prot);

  name = revm_resolve(bp->obj, addr, &off);
  snprintf(logbuf, BUFSIZ, XFMT, addr);
  hash_del(&e2dbgworld.bp, logbuf);

  if (off)
    snprintf(logbuf, BUFSIZ, " [*] Breakpoint at " AFMT 
	     " <%s + " UFMT "> removed\n\n", addr, name, off);
  else
    snprintf(logbuf, BUFSIZ, " [*] Breakpoint at " AFMT " <%s> removed\n\n", 
	 addr, name);
  e2dbg_output(logbuf);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}




