/*
** cmdapi.c for elfsh
** 
** Started on  Mon Feb 24 15:52:06 2003 mayhem
** Last update Thu Mar 11 14:39:06 2004 mayhem
**
** $Id: commands.c,v 1.3 2007-07-17 18:11:24 may Exp $
**
*/
#include "revm.h"



/* Create the cmdhandler passed to hash_add() */
revmcmd_t	*revm_create_CMDENT(int  (*exec)(void *file, void *av),
				  int  (*reg)(u_int index, u_int argc, char **argv),
				  int  flags,
				  char *help)
{
  revmcmd_t	*new;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  XALLOC(__FILE__, __FUNCTION__, __LINE__,new, sizeof(revmcmd_t), NULL);
  new->exec   = exec;
  new->reg    = reg;
  new->wflags = flags;
  new->help   = help;
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, (new));
}


/* Change the handler for an ELFsh command */
int		revm_command_set(char *cmd, void *exec, void *reg, u_int needcur)
{
  hashent_t	*ent;
  revmcmd_t	*act;
  char		logbuf[BUFSIZ];

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  ent = hash_get_ent(&cmd_hash, cmd);
  if (!ent)
    {
      snprintf(logbuf, BUFSIZ - 1,
	       "\n [!] Unknown command %s \n\n", world.curjob->curcmd->param[0]);
      revm_output(logbuf);
      PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
			"Supplied handler invalid", (-1));
    }
  act = ent->data;
  if (reg != ELFSH_ORIG)
    act->reg = reg;
  if (exec != ELFSH_ORIG)
    act->exec = exec;
  if (needcur != (u_int) ELFSH_ORIG)
    act->wflags = needcur;
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/* Add a command */
int		revm_command_add(char *cmd, void *exec, void *reg, u_int needfile, 
			  char *help)
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  hash_add(&cmd_hash, cmd , (void *) revm_create_CMDENT(exec, reg, needfile, help));
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}


/* Delete a command */
int		revm_command_del(char *cmd)
{

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  hash_del(&cmd_hash, cmd);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}