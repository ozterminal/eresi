/*
** mem.c for kernsh
** 
** $Id: mem.c,v 1.14 2008-02-16 12:32:26 thor Exp $
**
*/
#include "kernsh.h"
#include "libkernsh.h"

/* Handle the sys_call_table */
int		cmd_sct()
{
  int		ret;
  int		index;
  listent_t     *actual;
  list_t	*h;
  libkernshsyscall_t *sct;
  char		buff[BUFSIZ];

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  XALLOC(__FILE__, __FUNCTION__, __LINE__, h, sizeof(list_t), -1);
  elist_init(h, "cmd_sct_list", ASPECT_TYPE_UNKNOW);
  ret = kernsh_sct(h);

  memset(buff, '\0', sizeof(buff));
  snprintf(buff, sizeof(buff), 
	   "%s\n\n",
	   revm_colorfieldstr("[+] SYS_CALL_TABLE"));
  revm_output(buff);

  for (index = 0, actual = h->head; 
       index < h->elmnbr; 
       index++, actual = actual->next)
    {
      sct = (libkernshsyscall_t *) actual->data;
      memset(buff, '\0', sizeof(buff));
      snprintf(buff, sizeof(buff),
	       "%s %-40s %s %s\n", 
	       revm_colornumber("id:%-10u", (unsigned int)index),
	       revm_colortypestr_fmt("%s", sct->name),
	       revm_colorstr("@"),
	       revm_coloraddress(XFMT, (elfsh_Addr) sct->addr));
      revm_output(buff);
      revm_endline();
      
    }
  
  revm_output("\n");
  elist_destroy(h);

  if (ret)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Cannot get syscalltable", -1);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/* Handle the idt */
int		cmd_idt()
{
  int		ret;
  int		index;
  listent_t     *actual;
  list_t	*h;
  libkernshint_t *dint;
  char		buff[BUFSIZ];

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  XALLOC(__FILE__, __FUNCTION__, __LINE__, h, sizeof(list_t), -1);
  elist_init(h, "cmd_idt_list", ASPECT_TYPE_UNKNOW);
  ret = kernsh_idt(h);
        
  memset(buff, '\0', sizeof(buff));
  snprintf(buff, sizeof(buff), 
	   "%s\n\n",
	   revm_colorfieldstr("[+] IDT"));
  revm_output(buff);
  
  for (index = 0, actual = h->head; 
       index < h->elmnbr; 
       index++, actual = actual->next)
    {
      dint = (libkernshint_t *) actual->data;

      snprintf(buff, sizeof(buff),
	       "%s %-40s %s %s\n",
	       revm_colornumber("id:%-10u", (unsigned int)index),
	       revm_colortypestr_fmt("%s", dint->name),
	       revm_colorstr("@"),
	       revm_coloraddress(XFMT, (elfsh_Addr) dint->addr));
      revm_output(buff);
      revm_endline();
    }

  revm_output("\n");
  elist_destroy(h);

  if (ret)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Cannot get idt", -1);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/* Handle the gdt */
int		cmd_gdt()
{
  int		ret;
  int		index;
  int		i;
  listent_t     *actual;
  list_t	*h;
  libkernshsgdt_t *sgdt;
  char		buff[BUFSIZ];

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  XALLOC(__FILE__, __FUNCTION__, __LINE__, h, sizeof(list_t), -1);

  elist_init(h, "cmd_gdt_list", ASPECT_TYPE_UNKNOW);

  ret = kernsh_gdt(h);
        
  memset(buff, '\0', sizeof(buff));
  snprintf(buff, sizeof(buff), 
	   "%s\n\n",
	   revm_colorfieldstr("[+] GDT"));
  revm_output(buff);
  
  for (i = 0, index = 0, actual = h->head; 
       index < h->elmnbr; 
       i+=8, index++, actual = actual->next)
    {
      sgdt = (libkernshsgdt_t *) actual->data;
      
      snprintf(buff, sizeof(buff),
	       "%s%s %s %s\n",
	       revm_coloraddress("%.8lX", (elfsh_Addr) sgdt->deb),
	       revm_coloraddress("%.8lX", (elfsh_Addr) sgdt->fin),
	       revm_colorstr("@"),
	       revm_coloraddress(XFMT, (elfsh_Addr) sgdt->addr));

      revm_output(buff);
      
      revm_endline();
    } 
	     
  revm_output("\n");
  elist_destroy(h);

  if (ret)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Cannot get gdt", -1);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/* Handle kernel symbol */
int		cmd_ksym()
{
  char          *param;
  char		buff[BUFSIZ];
  unsigned long	addr;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  param = world.curjob->curcmd->param[0];

  if (param)
    {
      if(kernsh_get_addr_by_name(param, &addr, strlen(param)))
	{
	  revm_setvar_int("_", -1);
	  PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		       "Cannot get symbol", -1);
	}
      memset(buff, '\0', sizeof(buff));
      snprintf(buff, sizeof(buff), 
	       "%s %s %s %s %s\n\n",
	       revm_colorfieldstr("SYMBOL"),
	       revm_colorstr(param),
	       revm_colorfieldstr("is"),
	       revm_colorstr("@"),
	       revm_coloraddress(XFMT, (elfsh_Addr) addr));
      revm_output(buff);
      revm_setvar_long("_", addr);
    }

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/* Handle loadable kernel module */
int		cmd_kmodule()
{
  int	ret;
  char  *param, *param2, *param3, *param4;
  char	buff[BUFSIZ];

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  memset(buff, '\0', sizeof(buff));

  param = world.curjob->curcmd->param[0];
  param2 = world.curjob->curcmd->param[1];
  param3 = world.curjob->curcmd->param[2];
  param4 = world.curjob->curcmd->param[3];

  if (param)
    {
      if (param2 && param3 && !strcmp(param, "-r"))
	{
	  ret = kernsh_relink_module(param2, param3, param4);
	  if (ret == 0)
	    {
	      if (param4 == NULL)
		param4 = param2;
	      snprintf(buff, sizeof(buff), 
		       "Module %s and %s is linked in %s\n\n",
		       revm_colorstr(param2),
		       revm_colorstr(param3),
		       revm_colorstr(param4));
	      revm_output(buff);
	    }
	}
      else if (param2 && param3 && param4 && !strcmp(param, "-i"))
	{
	  ret = kernsh_infect_module(param2, param3, param4);
	  if (ret == 0)
	    {
	      snprintf(buff, sizeof(buff), 
		       "%s have been replaced by %s in %s\n\n",
		       revm_colorstr(param3),
		       revm_colorstr(param4),
		       revm_colorstr(param2));
	      revm_output(buff);
	    }
	}
    }

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, ret);
}

int		cmd_kmodule_load()
{
  int	ret;
  char  *filename;
  char	buff[BUFSIZ];

  ret = -1;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  filename = world.curjob->curcmd->param[0];

  if (filename)
    {
      ret = kernsh_kload_module(filename);
      if (ret == 0)
	{
	  snprintf(buff, sizeof(buff), 
		   "Module %s is loaded\n\n",
		   revm_colorstr(filename));
	  revm_output(buff);
	}
    }

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, ret);
}

int		cmd_kmodule_unload()
{
  int	ret;
  char  *filename;
  char	buff[BUFSIZ];

  ret = -1;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  filename = world.curjob->curcmd->param[0];

  if (filename)
    {
      ret = kernsh_kunload_module(filename);
      if (ret == 0)
	{
	  snprintf(buff, sizeof(buff), 
		   "Module %s is unloaded\n\n",
		   revm_colorstr(filename));
	  revm_output(buff);
	}
    }

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, ret);
}
