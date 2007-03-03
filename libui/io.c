/*
** io.c for elfsh
**
** Started on  Fri Mar  5 00:55:40 2004 mayhem
** Updated on  Fri Mar 11 18:47:41 2005 ym
*/
#include "libui.h"




/* Display str on all term */
int		vm_output_bcast(char *str)
{
  hashent_t	*actual;
  int		index;
  int		ret = 0;
  revmjob_t	*old;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  /* Saving current output parameters */
  old = world.curjob;

  /* Net's outputs */
  if (world.state.vm_net)
    {
      for (index = 0; index < world.jobs.size; index++)
        {
	  for (actual = &world.jobs.ent[index];
	       actual != NULL && actual->key != NULL;
	       actual = actual->next)
            {
	      if (!strcmp(actual->key, "local")    ||
		  !strcmp(actual->key, "net_init") ||
		  !strncmp(actual->key, "DUMP", 4) ||
		  !((revmjob_t *)actual->data)->active);
	      continue;

	      world.curjob = actual->data;
	      ret |= vm_output(str);
	      vm_flush();
            }
        }
    }

  /* stdout */
  if (world.state.vm_mode != ELFSH_VMSTATE_CMDLINE)
    {
      world.curjob = hash_get(&world.jobs, "local");
      ret |= vm_output(str);
    }

  /* Restore */
  world.curjob = old;

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, (ret));
}


/* Display the prompt */
int		vm_display_prompt()
{
  char		*buf;
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  buf = ((world.state.vm_mode == ELFSH_VMSTATE_IMODE ||
	  world.state.vm_mode == ELFSH_VMSTATE_DEBUGGER ||
	  (world.state.vm_net && world.curjob->io.type != ELFSH_IOSTD)) ?
	 vm_get_prompt() : "");
  vm_output(buf);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}


/* Reset lines counters and ignore output state */
int		vm_flush()
{
  int  lines = 80;
  int  cols = 200;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  /* Cache output only in IMODE/DEBUGGER mode */
  if (world.state.vm_mode != ELFSH_VMSTATE_IMODE &&
      world.state.vm_mode != ELFSH_VMSTATE_DEBUGGER)
    PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);

#ifdef USE_READLN
  rl_get_screen_size(&lines, &cols);
#endif

  /* We * 2 because color take a lot of place !!! */
  world.curjob->io.outcache.lines = lines * 2;
  world.curjob->io.outcache.cols  = cols * 2;
  world.curjob->io.outcache.nblines = lines;
  world.curjob->io.outcache.ignore  = 0;

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);

}

/* OUTPUT handler for stdout */
int		vm_output(char *str)
{
  char		*tmp;
  char		c;
  int		ret;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  vm_log(str);

  /* No -- more -- in some modes */
  if ((world.state.vm_mode != ELFSH_VMSTATE_IMODE &&
       world.state.vm_mode != ELFSH_VMSTATE_DEBUGGER)
      || world.curjob->io.type == ELFSH_IODUMP
      || !world.curjob->io.outcache.lines
      || world.curjob->sourced)
    PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,
	 (world.curjob->io.output(str)));

  /* Discard outputs */
  if (world.curjob->io.outcache.ignore)
    PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, -1);

  /* Counts lines */
  tmp = strchr(str, '\n');
  while (tmp)
    {
      world.curjob->io.outcache.nblines--;
      if (*tmp == '\0')
	break;
      tmp ++;
      tmp = strchr(tmp, '\n');
    }

  /* Do the output */
  ret = world.curjob->io.output(str);

  /* Is there any lines remaining ? */
  if (world.curjob->io.outcache.nblines < 0)
    {
      vm_flush();
      world.curjob->io.output("-- press enter for more ('q/n' to quit / next) --\n");
      vm_log("-- press enter for more ('q/n' to quit / next) --\n");

      /* We decided to discard further output (until next vm_flush) */
      if ((read(world.curjob->io.input_fd, &c, 1) == 1) && (c == 'q' || c == 'n'))
	{
	  if (c == 'q')
	    world.curjob->io.outcache.ignore = 1;
	  world.curjob->io.output("\n");
	  vm_log("\n");
	  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,  
			     (c == 'q' ? -1 : -2));
	}
    }

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,  ret);
}

/* Output without buffering/log */
int		vm_output_nolog(char *str)
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,
		     world.curjob->io.output(str));
}


/* ERR output function (stderr) */
int		vm_outerr(char *str)
{
  vm_log(str);
  fprintf(stderr, "%s", str);
  return (0);
}

/* Its lighter than flex ... */
char		**vm_input(int *argc)
{
  char		**argv;
  char		*buf;
  int		nbr;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  /* Read, and sanitize a first time to avoid blank lines */
  buf = ((world.state.vm_mode == ELFSH_VMSTATE_IMODE ||
	  world.state.vm_mode == ELFSH_VMSTATE_DEBUGGER) ?
	 vm_get_prompt() : "");
  buf = vm_getln(buf);
  if (!buf)
    PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,(NULL));

  /* Blank Line : return the last command */
  if (buf == (char *) ELFSH_VOID_INPUT)
    PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,
	 ((char **) ELFSH_VOID_INPUT));

  if (world.state.vm_mode != ELFSH_VMSTATE_SCRIPT &&
      world.curjob->io.type == ELFSH_IOSTD)
    {
#if defined(USE_READLN)
      if (world.curjob->io.buf[0] != '\0')
	{
	  vm_log(buf);
	  vm_log("\n\n");
	}
      else
	{
	  vm_log("\n");
	}
#else
      vm_log(buf);
      vm_log("\n\n");
#endif
#if defined(USE_READLN)
#if 0
      HISTORY_STATE	*state;
      HIST_ENTRY	*entry;
      int		i;

      printf ("[READLN] history : [%s]\n", buf);

      state = history_get_history_state();
      printf("== HISTORY STATE ==\n");
      printf("   offset : %d\n", state->offset);
      printf("   length : %d\n", state->length);
      printf("   size : %d\n", state->size);
      printf("===================\n");
#endif
      add_history(buf);
      using_history();
#if 0
      state = history_get_history_state();
      printf("== HISTORY STATE ==\n");
      printf("   offset : %d\n", state->offset);
      printf("   length : %d\n", state->length);
      printf("   size : %d\n", state->size);
      printf("===================\n");

      printf("-- entries --\n");
      for (i = 0; i <= state->length + 1; i++)
	{
	  entry = history_get(i);
	  if (entry)
	    printf("-- entry %d [%s] (%x) \n", i, entry->line, entry->line);
	  else
	    printf("-- entry %d NULL \n", i);
	}
      printf("-------------\n\n");
#endif
#endif
    }

  /* Save the line for future references */
  if (world.curjob->oldline)
    {
      vm_readline_free(world.curjob->oldline);
      world.curjob->oldline = NULL;
    }
  world.curjob->oldline = vm_readline_malloc(strlen(buf) + 1);
  strcpy(world.curjob->oldline, buf);

  /* Allocate the correct pointer array for argv */
  nbr = vm_findblanks(buf);
  argv = vm_doargv(nbr, (u_int *)argc, buf);

  /* Find and replace "\xXX" sequences, then return the array */
  vm_findhex(*argc, argv);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, (argv));
}



/* OUTPUT handler for stdout */
int		vm_stdoutput(char *str)
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  printf("%s", str);
  fflush(stdout);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,0);
}


/* INPUT handler for stdin */
char		*vm_stdinput()
{
  char		tmpbuf[BUFSIZ + 1];
  int		len;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

#if defined(USE_READLN)
  if (world.state.vm_mode == ELFSH_VMSTATE_SCRIPT)
    {
#endif

      for (len = 0; len < BUFSIZ; len++)
	switch (read(world.curjob->io.input_fd, tmpbuf + len, 1))
          {
	  case 0:
	    *tmpbuf = 0x00;
	    goto end;
	  case 1:
	    if (tmpbuf[len] == '\n')
	      {
		if (len == 0)
		  {
		    //    if(world.state.vm_mode == ELFSH_VMSTATE_SCRIPT)
		    //        return (vm_stdinput());
		    //    else
		    PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,
			 ((char *)ELFSH_VOID_INPUT));
		  }
		tmpbuf[len] = 0x00;
		goto end;
	      }
	    break;
	  default:
	    *tmpbuf = 0x00;
	    goto end;
          }


    end:
      PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,
	   (*tmpbuf ? strdup(tmpbuf) : NULL));
 

#if defined(USE_READLN)
    }
  else
    {
      /* input in progress */
      if (world.curjob->io.buf == NULL)
	PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,
	 ((char *) ELFSH_VOID_INPUT));

      /* CTRL-D case */
      if (world.curjob->io.buf == (char *) ELFSH_EXIT_INPUT)
	   PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,
	    (NULL));

      /* empty string */
      if (strlen(world.curjob->io.buf) == 0)
	{
	  /* XXX memory leak, this dup will never be freed */
	  if (world.curjob->oldline)
	   PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,
	    (strdup(world.curjob->oldline)));
 

	  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,
	   ((char *) ELFSH_VOID_INPUT));
	}

      PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,
	   (strdup(world.curjob->io.buf)));
 
    }
#endif

}



/* Initialize Input/Output hooks */
int		vm_initio()
{
  static int	done = 0;
  revmjob_t	*initial;

  NOPROFILER_IN();
  if (done)
    NOPROFILER_ROUT(0);
  done = 1;

  XALLOC(__FILE__, __FUNCTION__, __LINE__,initial, sizeof(revmjob_t), -1);
  memset(initial, 0, sizeof(revmjob_t));
  initial->io.type      = ELFSH_IOSTD;
  initial->io.input_fd  = 0;
  initial->io.input     = vm_stdinput;
  initial->io.output_fd = 1;
  initial->io.output    = vm_stdoutput;
  initial->active	= 1;
  initial->createtime = time(&initial->createtime);
  world.initial = world.curjob = initial;
  hash_init(&world.jobs, "jobs", 11, ASPECT_TYPE_UNKNOW);
  hash_add(&world.jobs, "local", initial);
  initial->name = strdup("local");
 
  hash_init(&initial->loaded,    
	    "initial_loaded_files", 
	    51, ASPECT_TYPE_UNKNOW);
  hash_init(&initial->dbgloaded,
	    "initial_dbgloaded_files",
	    11, ASPECT_TYPE_UNKNOW);

  profiler_setcolor(vm_endline, vm_colorinstr, vm_colorstr, 
		    vm_colorfieldstr, vm_colortypestr, vm_colorend, 
		    vm_colorwarn, vm_colorfunction, vm_colorfilename);
  profiler_setmorecolor(vm_coloradv, vm_colorinstr_fmt, vm_coloraddress,
			vm_colornumber, vm_colorstr_fmt, 
			vm_colorfieldstr_fmt, 
			vm_colortypestr_fmt, vm_colorwarn_fmt);
  NOPROFILER_ROUT(0);
}




/* Return the greatest socket from the elfsh_net_client_list and sock. */
int             elfsh_getmaxfd()
{
  int           ret;
  revmjob_t    *serv;

#if  defined(ELFSHNET)
  int           index;
  hashent_t     *actual;
#endif

  /* If the network is not UP, the biggest fd is 0 */
  serv = hash_get(&world.jobs, "net_init");
  if (serv == NULL)
    return (0);

  ret = serv->sock.socket;

#if  defined(ELFSHNET)
  /* remote elfsh (DUMP) */
  ret = (ret > dump_world.sock) ? ret : dump_world.sock;

  for (index = 0; index < dump_world.ports.size; index++)
    {
      for (actual = &dump_world.ports.ent[index];
           actual != NULL && actual->key != NULL;
           actual = actual->next)
        {
	  //	  if (!((revmjob_t *) actual->data)->active)
	  //	    continue;

#if __DEBUG_NETWORK__
          fprintf(stderr,
		  "[DEBUG NETWORK] Socket (DUMP) [%u] \n",
		  (int) actual->data);
#endif

	  if ((long) actual->data > ret)
	    ret = (long) actual->data;
	}
    }

  /* remote client */
  for (index = 0; index < world.jobs.size; index++)
    {
      for (actual = &world.jobs.ent[index];
           actual != NULL && actual->key != NULL;
           actual = actual->next)
        {

	  if (!((revmjob_t *) actual->data)->active)
	    continue;

#if __DEBUG_NETWORK__
          fprintf(stderr,
		  "[DEBUG NETWORK] Socket [%u] key = %10s ; next = %p\n",
		  ((revmjob_t *) actual->data)->sock.socket,
		  actual->key, actual->next);
#endif

          if (((revmjob_t *) actual->data)->sock.socket > ret)
            ret = ((revmjob_t *) actual->data)->sock.socket;
        }
    }
#endif

  return (ret);
}

/*
** Add a main socket and client's sockets to the sockets list used by select
** and call get_max_fd to get the greatest.
*/
int		elfsh_prepare4select(fd_set *sel_sockets)
{
  hashent_t     *actual;
  int           index;

#if defined(ELFSHNET)
  /* remote elfsh (DUMP) */
  for (index = 0; index < dump_world.ports.size; index++)
    {
      for (actual = &dump_world.ports.ent[index];
           actual != NULL && actual->key != NULL;
           actual = actual->next)
        {
	  //	  if (!((revmjob_t *) actual->data)->active)
	  //	    continue;

#if __DEBUG_NETWORK__
          fprintf(stderr, "[DEBUG NETWORK] prepare_4_select : (DUMP) socket : %d \n",
		  (int) actual->data);
#endif
          FD_SET((long) actual->data, sel_sockets);
        }
    }
#endif

  /* remote client */
  for (index = 0; index < world.jobs.size; index++)
    {
      for (actual = &world.jobs.ent[index];
           actual != NULL && actual->key != NULL;
           actual = actual->next)
        {

	  // Please revmjob_t *cur here instead of casting so much times
	  if (!((revmjob_t *) actual->data)->active)
	    continue;

#if _DEBUG_NETWORK__
	  fprintf(stderr, "[DEBUG NETWORK] prepare_4_select : socket : %d \n",
		  ((revmjob_t *)actual->data)->sock.socket);
#endif
#if defined(ELFSHNET)
	  if (((revmjob_t *) actual->data)->io.type == ELFSH_IODUMP)
	    continue;

	  if (((revmjob_t *) actual->data)->io.type == ELFSH_IONET)
	    FD_SET(((revmjob_t *) actual->data)->sock.socket, sel_sockets);
#endif

	  if (((revmjob_t *) actual->data)->io.type == ELFSH_IOSTD)
	    FD_SET(((revmjob_t *) actual->data)->io.input_fd, sel_sockets);
	}

    }
  return (elfsh_getmaxfd());
}

/* Set IO to the choosen socket */
int			vm_socket_getnew()
{
  revmjob_t		*cur;
  hashent_t		*actual;
  int			index;

  for (index = 0; index < world.jobs.size; index++)
    {
      for (actual = &world.jobs.ent[index];
           actual != NULL && actual->key != NULL;
           actual = actual->next)
        {
	  cur = (revmjob_t *) actual->data;
	  if (!cur || !cur->active)
	    continue;

	  if (cur->io.type == ELFSH_IODUMP &&
	      cur->io.new != 0)
	    {
	      world.curjob = cur;
	      return (1);
	    }

	  if (cur->io.type == ELFSH_IONET)
	    {

	      if (cur->sock.recvd_f == NEW &&
		  cur->sock.ready_f == YES)
		{
		  world.curjob = cur;
		  return (1);
		}
	    }
	}
    }
  return (0);
}

/* Clean jobs */
int                   vm_clean_jobs()
{
  hashent_t           *actual;
  int                 index;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  for (index = 0; index < world.jobs.size; index++)
    {
      for (actual = &world.jobs.ent[index];
           actual != NULL && actual->key != NULL;
           actual = actual->next)
        {

	  switch (((revmjob_t *) actual->data)->io.type)
	    {
	    case ELFSH_IODUMP:
#if defined(ELFSHNET)
	      if (((revmjob_t *) actual->data)->io.new == 0)
		{
#if __DEBUG_NETWORK__
		  printf("CLEAN : %s\n", actual->key);
#endif
		  /* if we delete jobs here, we do not have
		     session persistence and every load must be
		     done through shared load */

		  /* XXX can we free io.pkt ?? */

		  //		  dump_free(((revmjob_t *) actual->data)->io.pkt);
		  //		  XFREE(__FILE__, __FUNCTION__, __LINE__,((revmjob_t *) actual->data));
		  //		  hash_del(&world.jobs, actual->key);
		}
#endif
	      break;
	    default:
	      break;
	    }
        }
    }
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,0);
}

/* Return the current local job */
revmjob_t		*vm_get_curlocaljob()
{
  hashent_t		*actual;
  int			index;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  for (index = 0; index < world.jobs.size; index++)
    {
      for (actual = &world.jobs.ent[index];
           actual != NULL && actual->key != NULL;
           actual = actual->next)
        {
	  if (!((revmjob_t *) actual->data)->active)
	    continue;

	  if (((revmjob_t *) actual->data)->io.type == ELFSH_IOSTD)
	      PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,
		   actual->data);

	}
    }

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,(NULL));
}

/* Wait for all input */
int                     vm_select()
{
  fd_set		sel_sockets;
  int                   max_fd;
  int                   cont;
  revmjob_t            *init;
  int			err;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  /* By default do only one select. */
  init = hash_get(&world.jobs, "net_init");
  cont = 0;

  /* Flush pending outputs */
  vm_flush();

  /* In case of not already handle data */
  if (vm_socket_getnew())
    PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, (0));

  /* clean jobs hash table */
  vm_clean_jobs();

  do
    {
      FD_ZERO(&sel_sockets);

#if defined(ELFSHNET)
      if (world.state.vm_net && init)
        {
          // We add the serv_sock to the sockets list for select
          FD_SET(init->sock.socket, &sel_sockets);
	  /* add DUMP main socket */
	  FD_SET(dump_world.sock, &sel_sockets);
        }
#endif

      // Prepare for the select() call
      max_fd = elfsh_prepare4select(&sel_sockets);

      // In the case of normal loop print prompt
      if (cont == 0 &&
	  (world.state.vm_mode != ELFSH_VMSTATE_CMDLINE ||
	   world.state.vm_net))
        {
	  if (world.curjob->io.type != ELFSH_IODUMP)
	    {

	      // Display prompt
#if defined(USE_READLN)
	      if (world.curjob->io.type == ELFSH_IOSTD)
		{
		  if (world.curjob->io.buf != NULL) {
		      rl_forced_update_display();
		      vm_log(vm_get_prompt());
		    }
		}
	      else
#endif
		{
		  vm_display_prompt();
		}
	    }
        }

      // Reset cont if a loop has already be done
      cont = 0;
    retry:
      //printf("Trying select ... \n");
      err = select(max_fd + 1, &sel_sockets, NULL, NULL, NULL);
      if (err < 1 && errno == EINTR)
	{
	  //printf("Retrying select ... \n");
	  goto retry;
	}

      if (err > 0)
        {
#if defined(ELFSHNET)
          if (world.state.vm_net && init)
            {
              // Read net command if any.
              if (vm_net_recvd(&sel_sockets) < 0)
                fprintf(stderr, "vmnet_select : vm_net_recvd()"
			" failed\n");

              // Is there anybody out there (remote client) ?
              if (FD_ISSET(init->sock.socket, &sel_sockets))
                {
                  if (vm_net_accept() < 0)
                    fprintf(stderr, "Connection rejected\n");
                }

              // Is there anybody out there (DUMP) ?
              if (FD_ISSET(dump_world.sock, &sel_sockets))
                {
                  if (vm_dump_accept() < 0)
                    fprintf(stderr, "Connection rejected\n");
                }
            }
#endif
          if (world.state.vm_mode != ELFSH_VMSTATE_CMDLINE)
            {
              if (FD_ISSET(0, &sel_sockets))
                {
		  world.curjob = vm_get_curlocaljob();
		  if (!world.curjob)
		    PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,(-1));


#if defined (USE_READLN)
		  world.curjob->io.buf = NULL;

		  rl_callback_read_char();

		  if (world.curjob->io.buf != NULL)
                    {
		      /* restore prompt that will be display
		       * on next  rl_forced_update_display()
		       * or rl_callback_read_char() */
		      rl_restore_prompt();
		      PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, (0));
                    }
#endif
		  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,(0));
                }
            }

          // Select which command will be proceded
          if (world.state.vm_net)
            {
              if (vm_socket_getnew())
		  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,(0));
              else
                {
#if __DEBUG_NETWORK__
                  fprintf(stderr, "[DEBUG NETWORK] Select broken by a new connexion.\n");
#endif
                  // Let's re-select
                  cont = 1;
                }
            }
	}
    } while (cont);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__,(0));
}

/* readline line handler */
void    vm_ln_handler(char *c)
{

 PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

#if defined(USE_READLN)
  world.curjob->io.buf = c;
  if (c != NULL)
    {
      if (!c[0])
	vm_log("\n");
      /* save (remove) prompt if a complete line was typed
       * so that the line is not displayed on return */
      rl_save_prompt();
    }
  if (c == NULL)
    {
      /* special to enable exit on CTRL-D */
      world.curjob->io.buf = (char *) ELFSH_EXIT_INPUT;
      rl_save_prompt();
    }
#endif
 PROFILER_OUT(__FILE__, __FUNCTION__, __LINE__);
}


/* Change the Input file */
void	vm_setinput(revmjob_t *j, int fd)
{
  j->io.input_fd = fd;
}

/* Change the Input file */
void	vm_setoutput(revmjob_t *j, int fd)
{
  j->io.output_fd = fd;
}
