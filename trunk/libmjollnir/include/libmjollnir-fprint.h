/*
 * 2001-2006 Devhell Labs
 * 2006-2007 Asgard Labs, thorolf
 * 
 * Basic blocks related structures for libmjollnir
 *
 * $Id: libmjollnir-fprint.h,v 1.10 2007-02-26 17:47:15 thor Exp $
 *
 */
#include "libmjollnir-blocks.h"

#define		MJR_FPRINT_TYPE_MD5	0

/* Abstract function representation */
typedef struct		s_function 
{
  elfsh_Addr		vaddr;		/* Function virtual address */
  u_int			size;		/* Function size */
  char			name[64];	/* Function name */
  mjrblock_t		*first;		/* First function block */
  mjrlink_t		*parentfuncs;	/* Functions we are called from */
  u_int			parentnbr;	/* Number of parent functions */
  mjrlink_t		*childfuncs;	/* Functions we call */
  u_int			childnbr;	/* Number of child functions */
  char			md5[34];	/* MD5 Checksum */
}			mjrfunc_t;

#define ELFSH_SECTION_NAME_EDFMT_FUNCTIONS ".edfmt.functions"
#define ELFSH_SECTION_NAME_EDFMT_FCONTROL  ".edfmt.fcontrol"