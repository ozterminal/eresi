/*
** sht_rebuild.c for elfsh
** 
** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
** This code is functional as it, but it is WORK IN PROGRESS 
** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
** 
** Started on  Tue Mar 26 19:07:23 2002 mayhem
** Updated on  Thu Mar 23 23:21:08 2006 thorkill
** 
**
** $Id: sht_rebuild.c,v 1.10 2007-03-07 16:45:35 thor Exp $
**
*/
#include "libelfsh.h"

/** 
 * This function insert a SHT entry without shifting the address space
 * I.E. it does truncate some sections for creating new ones.
 *
 * This is necessary when we do SHT reconstruction or when we unmerge
 * the PLT section from the TEXT section on the MIPS architecture.
 */
int	elfsh_merge_shtentry(elfshobj_t *file, 
			     int	phdr_index, 
			     elfsh_Shdr shdr, 
			     char	*name)
{
  elfshsect_t	*sect;
  int		index;
  int		namelen;
  int		len;
  int		off;
  char		buff[BUFSIZ];
  void		*data;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  
  namelen = strlen(name);
  
  printf("Insertion section header *%s* \n", name);
  
  for (index = 0; index < file->hdr->e_shnum; index++)
    {
      /* Used to allow a 3 bytes padding for the current section */
      len = file->sht[index].sh_size - shdr.sh_size;
      
      printf("SHT rebuild: len = *%d* \n", len);
      
      if (len < 0)
	len = -len;
      
      /* The new section perfectly match an existing one (with an allowed padding of 3 bytes) */
      if (file->sht[index].sh_offset == shdr.sh_offset && len < 4)
	{
	  
	  /* In the rare case where both names have equal lenght, we can optimize a bit */
	  data = elfsh_get_raw(file->secthash[ELFSH_SECTION_SHSTRTAB]);
	  len = (file->sht[index].sh_name ? strlen(data + file->sht[index].sh_name) : 0);
	  if (len == namelen)
	    {
	      printf("SHT rebuild case 1.1 \n");
	      memcpy(data + file->sht[index].sh_name, name, len);
	    }
	  
	  /* The off variable is used just to clean the code . This case is the common one . */
	  else
	    {
	      printf("SHT rebuild case 1.2 \n");
	      off = file->sht[index].sh_name + len + 1;
	      memmove(data + file->sht[index].sh_name, 
		      data + off, 
		      file->secthash[ELFSH_SECTION_SHSTRTAB]->shdr->sh_size - off);
	      file->sht[index].sh_size -= len + 1;
	      elfsh_update_nameidx(file, file->sht[index].sh_name, len + 1);
	      file->sht[index].sh_name = elfsh_insert_in_shstrtab(file, name);
	    }
	  break;
	}
      
      
      /* The new header defines a section matching the beginning area of an existing section */
      else if (file->sht[index].sh_offset == shdr.sh_offset)
	{
	  printf("SHT rebuild case 2 \n");
	  XREALLOC(__FILE__, __FUNCTION__, __LINE__,file->sht, 
		   file->sht, (file->hdr->e_shnum + 1) * file->hdr->e_shentsize, -1);
	  memmove(file->sht + index + 1, 
		  file->sht + index, 
		  file->hdr->e_shentsize * (file->hdr->e_shnum - index));
	  
	  file->sht[index] = shdr;
	  if (strcmp(ELFSH_SECTION_NAME_BSS, name))
	    {
	      file->sht[index + 1].sh_size -= (shdr.sh_size);
	      file->sht[index + 1].sh_offset += (shdr.sh_size + 1);
	    }
	  file->sht[index + 1].sh_addr += (file->sht[index + 1].sh_addr ? shdr.sh_size : 0);
	  
	  if (file->hdr->e_shstrndx >= index)
	    file->hdr->e_shstrndx++;
	  file->secthash[ELFSH_SECTION_SHSTRTAB]->shdr = file->sht + file->hdr->e_shstrndx;
	  file->sht[index].sh_name = elfsh_insert_in_shstrtab(file, name);
	  
	  XALLOC(__FILE__, __FUNCTION__, __LINE__,sect, sizeof(*sect), -1);
	  if (elfsh_add_section(file, sect, index, NULL, ELFSH_SHIFTING_ABSENT) < 0)
	    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
			 "Unable to add section", -1);
	  
	  file->hdr->e_shnum++;
	  elfsh_update_linkidx(file, index, 1);
	  break;
	}
      
      
      /* The new header defines a section matching the end area of an existing section */
      else if (file->sht[index].sh_offset + file->sht[index].sh_size == shdr.sh_offset + shdr.sh_size)
	{
	  printf("SHT rebuild case 3 \n");
	  XREALLOC(__FILE__, __FUNCTION__, __LINE__,
		   file->sht, file->sht, (file->hdr->e_shnum + 1) * file->hdr->e_shentsize, -1);
	  memmove(file->sht + index + 2, file->sht + index + 1, 
		  file->hdr->e_shentsize * (file->hdr->e_shnum - index - 1));
	  
	  file->sht[index + 1] = shdr;
	  file->sht[index].sh_size -= shdr.sh_size;
	  
	  file->hdr->e_shstrndx++;
	  file->secthash[ELFSH_SECTION_SHSTRTAB]->shdr = file->sht + file->hdr->e_shstrndx;
	  file->sht[index + 1].sh_name = elfsh_insert_in_shstrtab(file, name);
	  
	  XALLOC(__FILE__, __FUNCTION__, __LINE__,sect, sizeof(*sect), -1);
	  if (elfsh_add_section(file, sect, index, NULL, ELFSH_SHIFTING_ABSENT) < 0)
	    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
			 "Unable to add section", -1);
	  
	  file->hdr->e_shnum++;
	  elfsh_update_linkidx(file, index + 1, 1);
	  break;
	}
      
      
      /* The new header defines a section strictly included in an already existing section */
      else if (file->sht[index].sh_offset < shdr.sh_offset && 
	       file->sht[index].sh_offset + file->sht[index].sh_size > shdr.sh_offset + shdr.sh_size)
	{
	  printf("SHT rebuild case 4 \n");
	  
	  XREALLOC(__FILE__, __FUNCTION__, __LINE__,
		   file->sht, file->sht, (file->hdr->e_shnum + 2) * file->hdr->e_shentsize, -1);
	  memmove(file->sht + index + 3, 
		  file->sht + index + 1, 
		  file->hdr->e_shentsize * (file->hdr->e_shnum - index - 1));
	  
	  file->sht[index + 1] = shdr;
	  file->sht[index + 2] = elfsh_create_shdr(0, file->sht[index].sh_type,
						   file->sht[index].sh_flags,
						   shdr.sh_addr + shdr.sh_size,
						   shdr.sh_offset + shdr.sh_size,
						   file->sht[index].sh_size - 
						   (shdr.sh_offset - file->sht[index].sh_offset) - 
						   shdr.sh_size,
						   0, 0, 0, 0);
	  file->sht[index].sh_size = shdr.sh_offset - file->sht[index].sh_offset;
	  
	  file->hdr->e_shstrndx += 2;
	  file->secthash[ELFSH_SECTION_SHSTRTAB]->shdr = file->sht + file->hdr->e_shstrndx;
	  
	  file->sht[index + 1].sh_name = elfsh_insert_in_shstrtab(file, name);
	  snprintf(buff, sizeof(buff), "%s_"UFMT, 
		   (file->sht[index + 2].sh_addr ? 
		    ELFSH_SECTION_NAME_MAPPED : ELFSH_SECTION_NAME_UNMAPPED),
		   file->sht[index + 2].sh_offset);
	  file->sht[index + 2].sh_name = elfsh_insert_in_shstrtab(file, buff);
	  
	  XALLOC(__FILE__, __FUNCTION__, __LINE__,sect, sizeof(*sect), -1);
	  if (elfsh_add_section(file, sect, index, 
				NULL, ELFSH_SHIFTING_ABSENT) < 0)
	    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
			 "Unable to add section", -1);
	  
	  XALLOC(__FILE__, __FUNCTION__, __LINE__,sect, sizeof(*sect), -1);
	  if (elfsh_add_section(file, sect, index + 2, 
				NULL, ELFSH_SHIFTING_ABSENT) < 0)
	    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
			 "Unable to add section", -1);
	  
	  file->hdr->e_shnum += 2;
	  elfsh_update_linkidx(file, index, 1);
	  break;
	}
    }       
  
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/** 
 * INTERNAL FUNCTION : Rebuild an initial SHT and 
 * create some sections based on the pht entries 
 */
static int		elfsh_init_sht(elfshobj_t *file, u_int num)
{
  elfsh_Shdr	shdr;
  elfshsect_t	*sect;
  char		name[256];
  u_int		index,idx;
  u_int		nnames,lnames,dnames;
  u_int		tlsnames,ehnames,snames;
  long		type, total, loffset;
  
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  
  file->hdr->e_shentsize = sizeof(elfsh_Shdr);
  file->hdr->e_shoff = file->fstat.st_size;
  
  for (total = idx = 0; idx < num; idx++)
    {
      switch (file->pht[idx].p_type) 
	{
	case PT_NOTE:
	case PT_LOAD:
	case PT_DYNAMIC:
	case PT_TLS:
	case PT_INTERP:
	case PT_GNU_EH_FRAME:
	case PT_GNU_STACK:
	  total++;
	  break;
	}
    }
  
  file->hdr->e_shnum = total + 1;
  
  /* Create the initial SHT */
  XALLOC(__FILE__, __FUNCTION__, __LINE__, file->sht, 
	 file->hdr->e_shentsize * (total + 1), -1);
  
  loffset = (sizeof(Elf32_Ehdr) + (sizeof(Elf32_Phdr) * file->hdr->e_phnum));
  
  /* rebuild the sht based on the pht entries */
  for(idx = index = 0; index < num; index++)
    {
      switch(file->pht[index].p_type)
	{
	case PT_NOTE: 
	  type = SHT_NOTE; 
	  break;
	case PT_DYNAMIC:
	  type = SHT_DYNAMIC;
	  break;
	case PT_LOAD: 
	case PT_GNU_EH_FRAME:
	case PT_GNU_STACK:
	case PT_TLS:
	case PT_INTERP:
	  type = SHT_PROGBITS; 
	  break;
	default: 
	  continue;
	}
      
      shdr = elfsh_create_shdr(0, type, 0, file->pht[index].p_vaddr,
			       loffset,
			       file->pht[index].p_filesz ? file->pht[index].p_filesz : 
			       sizeof(Elf32_Shdr),
			       0, 0, 
			       file->pht[index].p_align, 0);

      file->sht[idx] = shdr;
      
      /* store the type for later use when fixing up the name of this section */
      file->sht[idx].sh_name = file->pht[index].p_type;
      
      XALLOC(__FILE__, __FUNCTION__, __LINE__, sect, sizeof(elfshsect_t), -1);
      if (elfsh_add_section(file, sect, idx++, NULL, ELFSH_SHIFTING_NONE) < 0)
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
		     "Unable to add section", -1);
      
      loffset = (loffset + (!file->pht[index].p_filesz ? 
			    sizeof(Elf32_Shdr) : file->pht[index].p_filesz));
    }
  
  /* Insert the section header string table (.shstrtab) */
  shdr = elfsh_create_shdr(0, SHT_STRTAB, 0, 0, file->fstat.st_size, 
			   0, 0, 0, 0, 0);
  
  file->sht[idx] = shdr;
  
  XALLOC(__FILE__, __FUNCTION__, __LINE__,sect, sizeof(elfshsect_t), -1);
  if (elfsh_add_section(file, sect, idx, NULL, ELFSH_SHIFTING_NONE) < 0)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
		 "Unable to add section", -1);
  
  file->secthash[ELFSH_SECTION_SHSTRTAB] = sect;
  file->hdr->e_shstrndx = idx; 
  
  snames = ehnames = tlsnames = nnames = lnames = dnames = 0;
  /* fixup the section names */
  for(index = 0; index < total; index++)
    {
      switch(file->sht[index].sh_name) 
	{
	case PT_INTERP:
	  snprintf(name, sizeof(name), ELFSH_SECTION_NAME_INTERP);
	  break;
	case PT_LOAD:
	  snprintf(name, sizeof(name), ".load%d", lnames++);
	  break;
	case PT_NOTE:
	  snprintf(name, sizeof(name), ".note%d", nnames++);
	  break;
	case PT_TLS:
	  snprintf(name, sizeof(name), ".tls%d", tlsnames++);
	  break;
	case PT_DYNAMIC:
	  snprintf(name, sizeof(name), ".dynamic%d", dnames++);
	  break;
	case PT_GNU_EH_FRAME:
	  snprintf(name, sizeof(name), ".eh_frame%d", ehnames++);
	  break;
	case PT_GNU_STACK:
	  snprintf(name, sizeof(name), ".stack%d", snames++);
	  break;
	default: break;
	}
      file->sht[index].sh_name = elfsh_insert_in_shstrtab(file, name);
    }
  
  file->sht[idx].sh_name = elfsh_insert_in_shstrtab(file, 
						    ELFSH_SECTION_NAME_SHSTRTAB);
  
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

#if 0
/**
 * First round : Read PHT and guess .bss, .interp, .dynamic and .notes bounds 
 */
static void	sht_first_round(elfshobj_t *file, u_int num)
{
  /*
    elfsh_Phdr	*dyn;
    elfsh_Shdr	shdr;
  */
  u_int		index;
  int		flags;
  
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  
  for (index = 0; index < num; index++)
    {
      flags = SHF_ALLOC;
      flags |= (elfsh_segment_is_writable(file->pht + index)    ? SHF_WRITE     : 0);
      flags |= (elfsh_segment_is_executable(file->pht + index)  ? SHF_EXECINSTR : 0);
      switch (file->pht[index].p_type)
	{
	  
	  /* The dynamic segment */
	  
	case PT_DYNAMIC:
	  dyn = file->pht + index;
	  shdr = elfsh_create_section_header(0, SHT_DYNAMIC, flags , file->pht[index].p_vaddr,
					     file->pht[index].p_offset, file->pht[index].p_filesz, 
					     0, 0, 0, 0);
	  elfsh_merge_shtentry(file, index, shdr, ELFSH_SECTION_NAME_DYNAMIC);
	  break;
	  
	  
	  /* The interpretor segment */
	  
	case PT_INTERP:
	  shdr = elfsh_create_section_header(0, SHT_PROGBITS, flags, file->pht[index].p_vaddr,
					     file->pht[index].p_offset, file->pht[index].p_filesz, 
					     0, 0, 0, 0);
	  elfsh_merge_shtentry(file, index, shdr, ELFSH_SECTION_NAME_INTERP);
	  break;
	  
	  
	  /* This is the auxiliary information */
	  
	case PT_NOTE:
	  shdr = elfsh_create_section_header(0, SHT_NOTE, flags, file->pht[index].p_vaddr,
					     file->pht[index].p_offset, file->pht[index].p_filesz, 
					     0, 0, 0, 0);
	  elfsh_merge_shtentry(file, index, shdr, ELFSH_SECTION_NAME_NOTES_ABI);
	  break;
	  
	  
	  /* Rebuild the .bss */
	  
	case PT_LOAD:
	  if (file->pht[index].p_memsz != file->pht[index].p_filesz)
	    {
	      shdr = elfsh_create_section_header(0, SHT_NOBITS, flags, 
						 file->pht[index].p_vaddr + file->pht[index].p_filesz, 
						 file->pht[index].p_offset + file->pht[index].p_filesz,
						 file->pht[index].p_memsz - file->pht[index].p_filesz,
						 0, 0, 0, 0);
	      elfsh_merge_shtentry(file, index, shdr, ELFSH_SECTION_NAME_BSS);
	    }
	  break;
	  
	  
	default:
	  break;
	}
    }    
  
  PROFILER_OUT(__FILE__, __FUNCTION__, __LINE__);
}


/**
 * INTERNAL FUNCTION: guess .text, .data, .init, .fini, and .bss bounds 
 */
static void	sht_second_round(elfshobj_t *file, u_int num)
{
  /* elfsh_Phdr	*dyn; */
  elfsh_Shdr	shdr;
  u_int		index;
  int		flags;
  char		*name;
  char		i;
  
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
  
  /* Do not remove if first_round() is entirely working with section paddings */
  PROFILER_OUT(__FILE__, __FUNCTION__, __LINE__);
  
  for (index = 0; index < num; index++)
    {
      flags = SHF_ALLOC;
      flags |= (elfsh_segment_is_writable(file->pht + index)  ? SHF_WRITE     : 0);
      flags |= (elfsh_segment_is_executable(file->pht + index)? SHF_EXECINSTR : 0);
      switch (file->pht[index].p_type)
	{
	case PT_LOAD:
	  shdr = elfsh_create_shdr(0, SHT_PROGBITS, flags, 
				   file->pht[index].p_vaddr, 
				   file->pht[index].p_offset, 
				   file->pht[index].p_filesz, 
				   0, 0, 0, 0);
	  i = INTERVAL(file->pht[index].p_vaddr, file->hdr->e_entry, 
		       file->pht[index].p_vaddr + file->pht[index].p_filesz);
	  name = (i ? ELFSH_SECTION_NAME_TEXT : ELFSH_SECTION_NAME_DATA);
	  printf("MERGIN SHDR second round!\n");
	  elfsh_merge_shtentry(file, index, shdr, name);
	default:
	  break;
	}
    }
}
#endif

/**
 * Recreate the section header table examining the program header table 
 */
int		elfsh_rebuild_sht(elfshobj_t *file)
{
  int		num;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  /* Some preliminary stuffs */
  if (!elfsh_get_pht(file, &num) || elfsh_init_sht(file, num) < 0)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
		      "Unable to get PHT or to init SHT", -1);

  //  sht_first_round(file, num);
  //  sht_second_round(file, num);
	
  file->shtrb = 1;
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}