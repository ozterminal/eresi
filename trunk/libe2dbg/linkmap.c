/*
** linkmap.c for e2dbg
**    
** Started on  Tue Aug 16 09:38:03 2005 mayhem                                                                                                                   
**
** $Id: linkmap.c,v 1.4 2007-05-31 14:45:51 may Exp $
**
*/
#include "libe2dbg.h"


/* Load linkmap */
int			e2dbg_linkmap_load(char *name)
{
  static int		done	= 0;
  elfshsect_t		*got;
  elfsh_Addr		*linkmap_entry;
  void			*data;
  elfshlinkmap_t	*actual;
  char			*gotname;
  char			*ename;
  elfsh_Ehdr		*hdr;
  u_int			elftypenum;
  elfsh_Sym		*endsym;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  if (done)
    PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);    

#if __DEBUG_LINKMAP__
  fprintf(stderr, "[e2dbg] Starting Loading LINKMAP !! \n");
#endif

  e2dbg_setup_hooks();
  vm_config();

  /* Load debugged file */
  if (name)
    {

      /* No need to fill ET_EXEC base addr */
      if (!vm_is_loaded(name) && vm_load_file(name, 0, NULL) < 0)
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			  "Cannot load file", -1);
      
#if __DEBUG_LINKMAP__
      fprintf(stderr, "[e2dbg_linkmap_load] file %s loaded\n", name);
#endif
      
      world.curjob->current->linkmap = E2DBG_DYNAMIC_LINKMAP;
      world.curjob->current->running = 0;
    }
  
#if __DEBUG_LINKMAP__
  fprintf(stderr, "[e2dbg_linkmap_load] Before switch\n");
#endif
  
  /* Switch to obj 1 */
  if (vm_doswitch(1) < 0)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		 "Cannot switch on object 1", -1);    
  
#if __DEBUG_LINKMAP__
  fprintf(stderr, "[e2dbg_linkmap_load] After switch \n");
#endif
  
  /* Base address for PIE binaries have to be imported */
  if (world.curjob->current->hdr->e_type == ET_DYN &&
      !world.curjob->current->rhdr.base)
    {
#if __DEBUG_LINKMAP__
      fprintf(stderr, "[e2dbg_linkmap_load] Inside ET_DYN condition\n");
#endif

      endsym = elfsh_get_symbol_by_name(world.curjob->current, "_end");

      fprintf(stderr, "endsym = %08X \n", (elfsh_Addr) endsym);
      sleep(1);

#if __DEBUG_LINKMAP__
      fprintf(stderr, "[e2dbg_linkmap_load] Filling PIE base"
	      " (_end ondisk = %08X / _end in memory = %08X) ! \n",
	      endsym->st_value, e2dbgworld.syms.piebase);
#endif 

      world.curjob->current->rhdr.base = e2dbgworld.syms.piebase -
	endsym->st_value;
    }

  /* Get ALTGOT or GOT if we used LD_PRELOAD */
  if (!e2dbgworld.preloaded)
    {
      gotname = ELFSH_SECTION_NAME_ALTGOT;
      got = elfsh_get_section_by_name(world.curjob->current, 
				      gotname, NULL, NULL, NULL);
    }
  else
    got = elfsh_get_gotsct(world.curjob->current);

#if __DEBUG_LINKMAP__
  fprintf(stderr, "[e2dbg_linkmap_load] %s section at " XFMT "\n",
	  got->name, got->shdr->sh_addr);
  fprintf(stderr, "[e2dbg_linkmap_load] BASE = %08x\n", 
	  world.curjob->current->rhdr.base);
#endif
  
  
  /* Fix first file linkmap entry */
  if (world.curjob->current->linkmap == E2DBG_DYNAMIC_LINKMAP)
    {
      /* Fix first file linkmap entry */
      hdr = elfsh_get_hdr(world.curjob->current);
      if (!hdr)
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			  "Cannot get ELF header", -1);    
      elftypenum = elfsh_get_objtype(hdr);
      
      //fprintf(stderr, "[e2dbg_linkmap_load] after ELF header \n");

      /* Get ALTGOT entry */
      data          = elfsh_get_raw(got);

      //fprintf(stderr, "[e2dbg_linkmap_load] after get_raw (data = %08X) \n", data);

      linkmap_entry = elfsh_get_got_entry_by_index(data, 1);
      
      //fprintf(stderr, "[e2dbg_linkmap_load] after entry_by_index (linkmap_entry = %08x)\n",
      //      linkmap_entry);

#if defined(__FreeBSD__) || defined(__NetBSD__)
      world.curjob->current->linkmap = (elfshlinkmap_t *)
	&((Obj_Entry *) elfsh_get_got_val(linkmap_entry))->linkmap;
#else
      world.curjob->current->linkmap = (elfshlinkmap_t *) elfsh_get_got_val(linkmap_entry);
#endif
      
    }
  
#if __DEBUG_LINKMAP__
  else
    fprintf(stderr, "[e2dbg_linkmap_load] Linkmap was -NOT- dynamic\n");

  fprintf(stderr, "[e2dbg_linkmap_load] LINKMAP Found at " XFMT "\n", 
	 world.curjob->current->linkmap);
#endif
  
  vm_doswitch(1);

  
  /* now load all linkmap's files */
  for (actual = elfsh_linkmap_get_lprev(world.curjob->current->linkmap);
       actual != NULL; 
       actual = elfsh_linkmap_get_lprev(actual))
    {
      
#if __DEBUG_LINKMAP__
      fprintf(stderr, "[e2dbg_linkmap_load] Running on LINKMAP PREV " XFMT "\n", 
	     actual);
#endif
      
      ename = elfsh_linkmap_get_lname(actual);
      if (ename && *ename && !vm_is_loaded(ename))
	{
	  if (vm_load_file(ename, elfsh_linkmap_get_laddr(actual), 
			   world.curjob->current->linkmap) < 0)
	    e2dbg_output(" [EE] Loading failed");
	}      
    }

#if __DEBUG_LINKMAP__
  fprintf(stderr, "[e2dbg_linkmap_load] Running on LINKMAP NEXT\n");
#endif
  
  for (actual = elfsh_linkmap_get_lnext(world.curjob->current->linkmap);
       actual != NULL; 
       actual = elfsh_linkmap_get_lnext(actual))
    {

      ename = elfsh_linkmap_get_lname(actual);
     
#if __DEBUG_LINKMAP__
      fprintf(stderr, "[e2dbg_linkmap_load] Running on LINKMAP NEXT " XFMT " (%s baseaddr %08X) \n", 
	      actual, ename, actual->laddr);
#endif

      if (ename && *ename && !vm_is_loaded(ename))
	{
	  if (vm_load_file(ename, elfsh_linkmap_get_laddr(actual), 
			   world.curjob->current->linkmap) < 0)
	    e2dbg_output(" [EE] Loading failed");
	}      
    }

  /* Everything was OK */
  e2dbg_output("\n");
  //elfsh_set_debug_mode();
  vm_doswitch(1);
  done = 1;
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}



/* Dump the linkmap */
int		 e2dbg_linkmap_print(elfshobj_t *file)
{
  char		 logbuf[BUFSIZ];
  elfshlinkmap_t *actual;
  int		 i = 1;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!file)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid argument", -1);

  if (!file->linkmap)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "No linkmap", -1);

  e2dbg_output(" .::. Linkmap entries .::. \n");

  for (actual = elfsh_linkmap_get_lprev(file->linkmap); actual != NULL; 
       actual = elfsh_linkmap_get_lprev(actual), i++)
    {
      snprintf(logbuf, BUFSIZ - 1, 
	       " [%02u] addr : " XFMT " dyn : " XFMT " - %s\n", i, 
	       (elfsh_Addr) elfsh_linkmap_get_laddr(actual), 
	       (elfsh_Addr) elfsh_linkmap_get_lld(actual),
	       elfsh_linkmap_get_lname(actual));
      e2dbg_output(logbuf);
    }

  for (actual = file->linkmap; actual != NULL; 
       actual = elfsh_linkmap_get_lnext(actual), i++)
    {
      snprintf(logbuf, BUFSIZ - 1, 
	       " [%02u] addr : "XFMT" dyn : "XFMT" - %s\n", i, 
	       (elfsh_Addr) elfsh_linkmap_get_laddr(actual), 
	       (elfsh_Addr) elfsh_linkmap_get_lld(actual),
	       elfsh_linkmap_get_lname(actual));
      e2dbg_output(logbuf);
    }

  e2dbg_output("\n");
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}



/* Linkmap dump command */
int		cmd_linkmap()
{
  int		old;
  int		ret;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (elfsh_is_debug_mode())
    {
      if (!world.curjob)
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			  "No current workspace", -1);
      
      if (!world.curjob->current)
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			  "No current file", -1);
      
      old = world.curjob->current->id;

      /* switch to the main program file */
      vm_doswitch(1);
      ret = e2dbg_linkmap_print(world.curjob->current);
      vm_doswitch(old);
      PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, (ret));
    }
  else
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Debug mode only command", (-1));
}




