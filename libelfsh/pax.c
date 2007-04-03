/*
** pax.c for elfsh
** 
** Started on  Tue Feb 11 20:52:36 2003 mayhem
** Last update Tue Feb 18 06:17:27 2003 mayhem
**
** $Id: pax.c,v 1.3 2007-03-07 16:45:35 thor Exp $
**
*/
#include "libelfsh.h"


/**
 * Change the ELF header flags field 
 */
int		elfsh_set_paxflags(elfsh_Ehdr *hdr, elfsh_Addr flags)
{
  u_short	*pax_flags;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!hdr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid NULL parameter", -1);
  pax_flags   = (u_short *) ((char *) hdr->e_ident + EI_PAX);
  *pax_flags = (elfsh_Word) flags;
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}


/**
 * Return the ELF header flags field 
 */
elfsh_Word	elfsh_get_paxflags(elfsh_Ehdr *hdr)
{
  u_short	*pax_flags;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!hdr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid NULL parameter", -1);
  pax_flags   = (u_short *) ((char *) hdr->e_ident + EI_PAX);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, (*pax_flags));
}


/**
 * Change the section header table file offset 
 */
char		elfsh_set_pax_pageexec(elfsh_Ehdr *hdr, elfsh_Addr off)
{
  u_short	*pax_flags;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!hdr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid NULL parameter", -1);
  pax_flags   = (u_short *) ((char *) hdr->e_ident + EI_PAX);
  if (off)
    *pax_flags |= ELFSH_PAX_PAGEEXEC;
  else
    *pax_flags &= (~ELFSH_PAX_PAGEEXEC);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/**
 * Return the section header table file offset 
 */
char		elfsh_get_pax_pageexec(elfsh_Ehdr *hdr)
{
  u_short	*pax_flags;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!hdr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid NULL parameter", -1);
  pax_flags = (u_short *) ((char *) hdr->e_ident + EI_PAX);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, ((*pax_flags & ELFSH_PAX_PAGEEXEC) && 
	  !(*pax_flags & ELFSH_PAX_SEGMEXEC)));
}



/**
 * Change the section header table file offset 
 */
char		elfsh_set_pax_emultramp(elfsh_Ehdr *hdr, elfsh_Addr off)
{
  u_short	*pax_flags;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!hdr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid NULL parameter", -1);
  pax_flags = (u_short *) ((char *) hdr->e_ident + EI_PAX);
  if (off)
    *pax_flags |= ELFSH_PAX_EMULTRAMP;
  else
    *pax_flags &= (~ELFSH_PAX_EMULTRAMP);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/**
 * Return the section header table file offset 
 */
char		elfsh_get_pax_emultramp(elfsh_Ehdr *hdr)
{
  u_short	*pax_flags;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!hdr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid NULL parameter", -1);
  pax_flags   = (u_short *) ((char *) hdr->e_ident + EI_PAX);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 
		     ((*pax_flags & ELFSH_PAX_EMULTRAMP)));
}

/**
 * Change the section header table file offset 
 */
char		elfsh_set_pax_mprotect(elfsh_Ehdr *hdr, elfsh_Addr off)
{
  u_short	*pax_flags;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!hdr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid NULL parameter", -1);
  pax_flags   = (u_short *) ((char *) hdr->e_ident + EI_PAX);
  if (!off)
    *pax_flags |= ELFSH_PAX_MPROTECT;
  else
    *pax_flags &= (~ELFSH_PAX_MPROTECT);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/**
 * Return the section header table file offset 
 */
char		elfsh_get_pax_mprotect(elfsh_Ehdr *hdr)
{
  u_short	*pax_flags;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!hdr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid NULL parameter", -1);
  pax_flags   = (u_short *) ((char *) hdr->e_ident + EI_PAX);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, (!(*pax_flags & ELFSH_PAX_MPROTECT)));
}


/** 
 * Change the section header table file offset 
 */
char		elfsh_set_pax_randmmap(elfsh_Ehdr *hdr, elfsh_Addr off)
{
  u_short	*pax_flags;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!hdr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid NULL parameter", -1);
  pax_flags   = (u_short *) ((char *) hdr->e_ident + EI_PAX);
  if (!off)
    *pax_flags |= ELFSH_PAX_RANDMMAP;
  else
    *pax_flags &= (~ELFSH_PAX_RANDMMAP);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/** 
 * Return the section header table file offset 
 */
char		elfsh_get_pax_randmmap(elfsh_Ehdr *hdr)
{
  u_short	*pax_flags;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!hdr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid NULL parameter", -1);
  pax_flags   = (u_short *) ((char *) hdr->e_ident + EI_PAX);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 
		     (!(*pax_flags & ELFSH_PAX_RANDMMAP)));
}


/**
 * Change the section header table file offset 
 */
char		elfsh_set_pax_randexec(elfsh_Ehdr *hdr, elfsh_Addr off)
{
  u_short	*pax_flags;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!hdr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid NULL parameter", -1);
  pax_flags   = (u_short *) ((char *) hdr->e_ident + EI_PAX);
  if (off)
    *pax_flags |= ELFSH_PAX_RANDEXEC;
  else
    *pax_flags &= (~ELFSH_PAX_RANDEXEC);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/**
 * Return the section header table file offset 
 */
char		elfsh_get_pax_randexec(elfsh_Ehdr *hdr)
{
  u_short	*pax_flags;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!hdr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid NULL parameter", -1);
  pax_flags   = (u_short *) ((char *) hdr->e_ident + EI_PAX);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, ((*pax_flags & ELFSH_PAX_RANDEXEC)));
}


/**
 * Change the section header table file offset 
 */
char		elfsh_set_pax_segmexec(elfsh_Ehdr *hdr, elfsh_Addr off)
{
  u_short	*pax_flags;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!hdr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid NULL parameter", -1);
  pax_flags   = (u_short *) ((char *) hdr->e_ident + EI_PAX);
  if (!off)
    *pax_flags |= ELFSH_PAX_SEGMEXEC;
  else
    *pax_flags &= (~ELFSH_PAX_SEGMEXEC);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/**
 * Return the section header table file offset 
 */
char		elfsh_get_pax_segmexec(elfsh_Ehdr *hdr)
{
  u_short	*pax_flags;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  if (!hdr)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		      "Invalid NULL parameter", -1);
  pax_flags   = (u_short *) ((char *) hdr->e_ident + EI_PAX);
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 
		     (!(*pax_flags & ELFSH_PAX_SEGMEXEC)));
}