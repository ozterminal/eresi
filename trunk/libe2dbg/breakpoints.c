/*
** breakpoints.c for e2dbg
**    
** Started on  Tue Aug 16 09:38:03 2005 mayhem                                                                                                                   
**
** $Id: breakpoints.c,v 1.2 2007-03-07 16:45:35 thor Exp $
**
*/
#include "libe2dbg.h"


/* Simple shared flags for watch/breakpoints */
static u_char	watchflag;



/* Add a breakpoint without using a script command */
int		e2dbg_breakpoint_add(elfsh_Addr addr, u_char flags)
{
  int		err;
  char		buf[BUFSIZ];
  char		*name;
  elfsh_SAddr	off;
  elfshobj_t	*file;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  /* Resolve source file */
  file = e2dbg_get_parent_object(addr);
  if (file == NULL)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
		      "Cannot resolve parent file for bp", -1);

  /* Resolve breakpoint address */
  name = vm_resolve(file, addr, &off);
  if (off)
    snprintf(buf, BUFSIZ, "<%s + " DFMT ">", name, off);
  else
    snprintf(buf, BUFSIZ, "<%s>", name);

  /* Really put the breakpoint */
  err = elfsh_bp_add(&e2dbgworld.bp, file, buf, addr, flags);
  if (err < 0)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Cannot add breakpoint", -1);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}



/* Return 1 if the breakpoint is a watchpoint */
int		e2dbg_is_watchpoint(elfshbp_t *b)
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 
		     b && b->flags & ELFSH_BP_WATCH);
}





/* Find breakpoint by ID */
elfshbp_t	*e2dbg_breakpoint_from_id(uint32_t bpid)
{
  hashent_t     *actual;
  elfshbp_t	*cur;
  int           index;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  
  for (index = 0; index < e2dbgworld.bp.size; index++)
    for (actual = e2dbgworld.bp.ent + index;
	 actual != NULL && actual->key != NULL;
	 actual = actual->next)
      {
	cur = (elfshbp_t *) actual->data;
	if (cur->id == bpid)
	  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 
			     cur);
      }

  PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
                    "Unable to find breakpoing by ID", NULL);
}



/* Find a breakpoint by various ways */
elfshbp_t	*e2dbg_breakpoint_lookup(char *name)
{
  elfsh_Addr	addr;
  elfsh_Sym	*sym;
  elfshbp_t	*bp;
  uint16_t	bpid;
  char		straddr[32];
  char		logbuf[BUFSIZ];

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  bp = NULL;
  
  /* Lookup by vaddr */
  if (IS_VADDR(name))
    {
      
      if (sscanf(name + 2, AFMT, &addr) != 1)
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
			  "Invalid virtual address requested", 
			  NULL);
    }

  /* Try to lookup by ID */
  else if (vm_isnbr(name))
    {
      bpid = atoi(name);
      bp   = e2dbg_breakpoint_from_id(bpid);
      if (!bp)
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
                          "Invalid breakpoint ID", NULL);
    }

  /* Resolve symbol */
  /* Sometimes we fix symbols on the disk only */
  /* This avoid a mprotect */
   else
    {
      sym = elfsh_get_metasym_by_name(world.curjob->current, 
				      name);
      if (!sym || !sym->st_value)
	{
	  elfsh_toggle_mode();
	  sym = elfsh_get_metasym_by_name(world.curjob->current,
					  name);
	  elfsh_toggle_mode();
	}
      if (!sym)
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
			  "No symbol by that name in the current file", 
			  NULL);
      
      if (!sym->st_value)
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
			  "Requested symbol address unknown",
			  NULL);
      addr = sym->st_value;
    }

  /* Get the breakpoint */
  if (!bp)
    {
      snprintf(straddr, sizeof(straddr), XFMT, addr);
      bp = hash_get(&e2dbgworld.bp, straddr);
      if (!bp)
	{
	  snprintf(logbuf, BUFSIZ, 
		   "\n [!] No breakpoint set at addr " AFMT " \n\n", addr);
	  e2dbg_output(logbuf);
	  PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			    "No breakpoint at this address", NULL);
	}
    }

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, bp);
}





/* Breakpoint command */
int		cmd_bp()
{
  char		*str;
  int		ret;
  elfsh_Addr	addr;
  elfsh_Sym	*sym;
  char		logbuf[BUFSIZ];
  int		idx;
  int		index;
  int		idx2 = 0;
  elfsh_SAddr	off = 0;
  char		*name;
  hashent_t	*actual;
  elfshbp_t	*cur;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  /* build argc */
  for (idx = 0; world.curjob->curcmd->param[idx] != NULL; idx++);
  str = vm_lookup_string(world.curjob->curcmd->param[0]);

  /* List breakpoints */
  if (idx == 0)
    {  
      e2dbg_output(" .:: Breakpoints & Watchpoints ::.\n\n");	      
      for (index = 0; index < e2dbgworld.bp.size; index++)
	{
	  for (actual = e2dbgworld.bp.ent + index; 
	       actual != NULL && actual->key != NULL; 
	       actual = actual->next)
	    {
	      idx2++;
	      cur = ((elfshbp_t *) actual->data);
	      name = vm_resolve(world.curjob->current, 
				(elfsh_Addr) cur->addr, &off);
	      
	      if (off)
		snprintf(logbuf, BUFSIZ, " %c [%02u] " XFMT " <%s + " UFMT ">\n", 
			 (e2dbg_is_watchpoint(cur) ? 'W' : 'B'),
			 cur->id, cur->addr, name, off);
	      else
		snprintf(logbuf, BUFSIZ, " %c [%02u] " XFMT " <%s>\n", 
			 (e2dbg_is_watchpoint(cur) ? 'W' : 'B'),
			 cur->id, cur->addr, name);
		  
	      e2dbg_output(logbuf);
	    }
	}

      if (!idx2)
	e2dbg_output(" [*] No breakpoints\n");
      
      e2dbg_output("\n");
    }

  /* Supply a new breakpoint */
  else if (idx == 1)
    {
      if (!elfsh_is_debug_mode())
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
			  "Not in dynamic or debugger mode", -1);
  
      if (!str || !(*str))
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			  "Invalid argument", -1);

      /* Break on a supplied virtual address */
      if (IS_VADDR(str))
	{
	  if (sscanf(str + 2, AFMT, &addr) != 1)
	    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			      "Invalid virtual address requested", (-1));
	}

      /* Resolve first a function name */
      else
	{
	  sym = elfsh_get_metasym_by_name(world.curjob->current, str);

	  /* Sometimes we fix symbols on the disk */
	  /* We avoid a mprotect */
	  if (!sym || !sym->st_value)
	    {
	      elfsh_toggle_mode();
	      sym = elfsh_get_metasym_by_name(world.curjob->current, str);
	      elfsh_toggle_mode();
	    }
	  
	  if (!sym)
	    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			      "No symbol by that name in the current file", (-1));
	  if (!sym->st_value)
	    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			      "Requested symbol address unknown", (-1));

	  addr = sym->st_value;
	  if (elfsh_get_objtype(world.curjob->current->hdr) == ET_DYN)
	    {
#if __DEBUG_BP__
	      printf(" [*] Adding base addr %08X \n", world.curjob->current->rhdr.base);
#endif
	      addr += world.curjob->current->rhdr.base;
	    }

#if __DEBUG_BP__
	  printf(" [*] Set breakpoint on %08X \n", addr);
#endif

	}

      /* Add the breakpoint */
      ret = e2dbg_breakpoint_add(addr, watchflag);
      if (ret < 0)
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			  "Breakpoint insertion failed\n", (-1));
      if (ret >= 0)
	{
	  name = vm_resolve(world.curjob->current, addr, &off);
	  if (!off)
	    snprintf(logbuf, BUFSIZ - 1, 
		     " [*] %spoint added at <%s> (" XFMT ")\n\n", 
		     (watchflag ? "Watch" : "Break"), name, addr);
	  else
	    snprintf(logbuf, BUFSIZ - 1, 
		     " [*] %spoint added at <%s + " UFMT "> (" XFMT ")\n\n", 
		     (watchflag ? "Watch" : "Break"), name, off, addr);
	  e2dbg_output(logbuf);
	}
    } 
  else 
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Wrong arg number", (-1));
  
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, (ret));
}



/* Watchpoint command */
int		cmd_watch()
{
  int		err;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  watchflag = 1;
  err = cmd_bp();
  watchflag = 0;
  if (err < 0)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
                      "Failed to install watchpoint", -1);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 
		     (err));
}
