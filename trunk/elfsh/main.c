/*
** main.c for elfsh
**
** Started on  Wed Feb 21 22:02:36 2001 mayhem
** Updated on  Wed Jan 03 17:51:04 2007 mayhem
**
** $Id: main.c,v 1.16 2007-03-07 16:45:34 thor Exp $
**
*/
#include "elfsh.h"

/* The real main function */
int		vm_main(int ac, char **av)
{
  vm_setup(ac, av, 0, 0);
  if (world.state.vm_mode != REVM_STATE_CMDLINE)
    vm_print_banner(av[1]);
  vm_config();
  setup_local_cmdhash();
  vm_output(" [*] Type help for regular commands \n\n");
  return (vm_run(ac, av));
}


/* The main ELFsh routine */
int		main(int ac, char **av)
{
  return (vm_main(ac, av));
}


