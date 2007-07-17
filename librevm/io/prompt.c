/*
** prompt.c for librevm in ERESI
**
** Interface for prompt management
**
** Started on  Fri Nov  2 15:21:56 2001 mayhem
** Updated on  Mon Mar  5 17:26:11 2007 mayhem
**
** $Id: prompt.c,v 1.6 2007-07-17 18:11:25 may Exp $
**
*/
#include "revm.h"


//char buf[BUFSIZ];


void    revm_set_prompt(void (*func) (char *name, u_int size))
{
  prompt_token_setup = func;
}

void	revm_create_default_prompt(char *name, u_int size)
{
  snprintf(name, size - 1,
	   "%s%s%s%s%s%s%s%s%s%s%s ",
	   revm_colorget("%s", "pspecial", "("),
	   (world.state.revm_mode == REVM_STATE_DEBUGGER ?
	    revm_colorget("%s", "psname" , E2DBG_ARGV0)    :
	    revm_colorget("%s", "psname" , REVM_NAME)),
	   revm_colorget("%s", "pspecial", "-"),
	   revm_colorget("%s", "pversion", REVM_VERSION),
	   revm_colorget("%s", "pspecial", "-"),
	   revm_colorget("%s", "prelease", REVM_RELEASE),
	   revm_colorget("%s", "pspecial", "-"),
	   revm_colorget("%s", "pedition", REVM_EDITION),
	   revm_colorget("%s", "pspecial", "@"),
	   revm_colorget("%s", "psname", world.curjob->ws.name),
	   revm_colorget("%s", "pspecial", ")"));
  revm_endline();
}

/* return the right prompt */
char*	revm_get_prompt()
{
  if (world.state.revm_mode == REVM_STATE_INTERACTIVE ||
      world.state.revm_mode == REVM_STATE_DEBUGGER)
    {
      /* Setup prompt only once */
      if (prompt_token_setup == NULL)
	revm_set_prompt(revm_create_default_prompt);

      if (prompt_token_setup)
	prompt_token_setup(prompt_token, sizeof(prompt_token));
      else
	snprintf(prompt_token, sizeof(prompt_token) - 1, "prompt-error");

      /* Prompt on readline need some modifications */
#if defined(USE_READLN)
      readln_prompt_update(prompt_token, sizeof(prompt_token));
#endif

      return prompt_token;
    }

  if (world.state.revm_mode == REVM_STATE_SCRIPT)
    return "";

  return "UNKNOWN MODE> ";
}


/* Display the prompt */
int		revm_display_prompt()
{
  char		*buf;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  buf = ((world.state.revm_mode == REVM_STATE_INTERACTIVE ||
	  world.state.revm_mode == REVM_STATE_DEBUGGER ||
	  (world.state.revm_net && world.curjob->ws.io.type != REVM_IO_STD)) ?
	 revm_get_prompt() : "");
  revm_output(buf);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}
