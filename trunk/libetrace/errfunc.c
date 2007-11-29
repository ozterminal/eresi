/**
 * @file errfunc.c
** errfunc.c for libetrace
**
** This file contain one function for error collection
**
**
** $Id: errfunc.c,v 1.2 2007-11-29 10:25:02 rival Exp $
**
*/

#include "libetrace.h"
#include "libetrace-extern.h"

/**
 * When an error occur in etrace. This function is used to show buggy function name.
 * @return function name
 */
char			*etrace_geterrfunc()
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__); 

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, last_parsed_function);
}
