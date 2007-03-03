/*
** tables.c for elfsh
**
** This file contains all command and objects definitions for scripting
**
** Started on  Sat Jan 25 07:48:41 2003 mayhem
*/
#include "revm.h"



/* Labels hash */
hash_t		labels_hash[ELFSH_MAX_SOURCE_DEPTH];

/* Variables hash */
hash_t		vars_hash;

/* The command hash table : hash the command name and returns a revmcmd_t */
hash_t		cmd_hash;

/* The constants hash tables : ascii defines from elf.h and revm.h */
hash_t		const_hash;

/* The ELF shell module hash tables: returns a revmmod_t */
hash_t		mod_hash;

/* The file hash table : hash the filename and returns a elfshobj_t pointer */
hash_t		file_hash;

/* The Level 1 object hash table : hash the object name and returns a revmL1_t pointer */
hash_t		L1_hash;

/* The Level 2 object hash table list : hash the object name and returns a revmL2_t pointer */
hash_t		elf_L2_hash;	/* For the ELF header fields */
hash_t		sht_L2_hash;	/* For the Sections/SHT objects */
hash_t		pht_L2_hash;	/* For the Program header table objects */
hash_t		sym_L2_hash;	/* For symbol object field */
hash_t		rel_L2_hash;	/* For relocation entry fields */
hash_t		dynsym_L2_hash;	/* For .dynsym objects */
hash_t		dyn_L2_hash;	/* For .dynamic objects */
hash_t		sct_L2_hash;	/* Sections names and data */
hash_t		got_L2_hash;	/* GOT objects */
hash_t          vers_L2_hash;   /* For .gnu.version */
hash_t          verd_L2_hash;   /* For .gnu.version_d */
hash_t          vern_L2_hash;   /* For .gnu.version_r */
hash_t          hashb_L2_hash;  /* For .hash (bucket) */
hash_t          hashc_L2_hash;  /* For .hash (chain) */

/* Color hash */
hash_t          fg_color_hash;
hash_t          bg_color_hash;
hash_t          t_color_hash;

/* Grammar parser hash */
hash_t		parser_hash;

/* Trace hash */
hash_t		traces_cmd_hash;

/* Lib path store variable */
char	       elfsh_libpath[BUFSIZ];



/* Fill all the Level 1 Objects hash tables */
static void	setup_L1hash()
{
  hash_init(&L1_hash, "L1objects", 29, ASPECT_TYPE_HASH);
  hash_add(&L1_hash, "hdr"     , (void *) vm_create_L1ENT(elfsh_get_hdr,
							  NULL, NULL,
							  &elf_L2_hash,
							  NULL, NULL, NULL, 0));

  hash_add(&L1_hash, "pht"     , (void *) vm_create_L1ENT(elfsh_get_pht,
							  NULL, NULL,
							  &pht_L2_hash,
							  elfsh_get_pht_entry_by_index,
							  NULL, NULL, 
							  sizeof (elfsh_Phdr)));

  hash_add(&L1_hash, "rpht"    , (void *) vm_create_L1ENT(elfsh_get_rpht,
							  NULL, NULL,
							  &pht_L2_hash,
							  elfsh_get_pht_entry_by_index,
							  NULL, NULL, 
							  sizeof (elfsh_Phdr)));


  hash_add(&L1_hash, "symtab"  , (void *) vm_create_L1ENT(elfsh_get_symtab,
							  NULL,
							  elfsh_get_symbol_by_name,
							  &sym_L2_hash,
							  elfsh_get_symbol_by_index,
							  NULL, NULL,
							  sizeof (elfsh_Sym)));

  hash_add(&L1_hash, "dynsym"  , (void *) vm_create_L1ENT(elfsh_get_dynsymtab,
							  NULL,
							  elfsh_get_dynsymbol_by_name,
							  &dynsym_L2_hash,
							  elfsh_get_symbol_by_index,
							  NULL, NULL,
							  sizeof (elfsh_Sym)));

  hash_add(&L1_hash, "got"     , (void *) vm_create_L1ENT(elfsh_get_got,
							  elfsh_get_got_by_idx,  
							  elfsh_get_got_entry_by_name,
							  &got_L2_hash,
							  elfsh_get_got_entry_by_index,
       							  elfsh_get_got_entry,
							  elfsh_set_got_entry,
							  sizeof (elfsh_Addr)));


  hash_add(&L1_hash, "dynamic" , (void *) vm_create_L1ENT(elfsh_get_dynamic,
							  NULL, NULL,
							  &dyn_L2_hash,
							  elfsh_get_dynamic_entry_by_index,
							  NULL, NULL,
							  sizeof (elfsh_Dyn)));

  hash_add(&L1_hash, "ctors"   , (void *) vm_create_L1ENT(elfsh_get_ctors,
							  NULL, 
							  elfsh_get_ctors_entry_by_name,
							  NULL,
							  elfsh_get_ctors_entry_by_index,
							  elfsh_get_ctors_entry,
							  elfsh_set_ctors_entry,
							  sizeof (elfsh_Addr)));

  hash_add(&L1_hash, "dtors"   , (void *) vm_create_L1ENT(elfsh_get_dtors,
							  NULL, 
							  elfsh_get_dtors_entry_by_name,
							  NULL,
							  elfsh_get_dtors_entry_by_index,
							  elfsh_get_dtors_entry,
							  elfsh_set_dtors_entry,
							  sizeof (elfsh_Addr)));
  
  hash_add(&L1_hash, "rel"     , (void *) vm_create_L1ENT(NULL,
							  elfsh_get_reloc, // slot 2
							  elfsh_get_relent_by_name,
							  &rel_L2_hash,
							  elfsh_get_relent_by_index,
							  NULL, NULL,
							  sizeof (elfsh_Rel)));
  
  hash_add(&L1_hash, "sht"     , (void *) vm_create_L1ENT(elfsh_get_sht,
							  NULL,
							  elfsh_get_sht_entry_by_name,
							  &sht_L2_hash,
							  elfsh_get_sht_entry_by_index,
							  NULL, NULL,
							  sizeof (elfsh_Shdr)));

  hash_add(&L1_hash, "rsht"     , (void *) vm_create_L1ENT(elfsh_get_runtime_sht,
							   NULL,
							   elfsh_get_sht_entry_by_name,
							   &sht_L2_hash,
							   elfsh_get_sht_entry_by_index,
							   NULL, NULL,
							   sizeof (elfsh_Shdr)));
  
  hash_add(&L1_hash, "section", (void *) vm_create_L1ENT(elfsh_get_section_list, 
							 NULL,
							 elfsh_get_section_by_nam,
							 &sct_L2_hash,
							 elfsh_get_section_by_idx,
							 NULL, NULL,
							 sizeof (elfshsect_t)));

  hash_add(&L1_hash, "version", (void *) vm_create_L1ENT(elfsh_get_versymtab,
							 NULL,
							 elfsh_get_versym_by_name,
							 &vers_L2_hash,
							 elfsh_get_versym_entry_by_index,
							 NULL, NULL,
							 sizeof (elfsh_Half)));
  
  hash_add(&L1_hash, "verdef", (void *) vm_create_L1ENT(elfsh_get_verdeftab,
							elfsh_get_verdeftab_by_idx,
							NULL,
							&verd_L2_hash,
							elfsh_get_verdef_entry_by_index,
							NULL, NULL,
							sizeof (elfsh_Verdef)));
  
  hash_add(&L1_hash, "verneed", (void *) vm_create_L1ENT(elfsh_get_verneedtab,
							 NULL,
							 elfsh_get_verneed_by_name,
							 &vern_L2_hash,
							 elfsh_get_verneed_entry_by_index, 
							 NULL, NULL,
							 sizeof (elfsh_Verneed)));

  hash_add(&L1_hash, "hashbucket", (void *) vm_create_L1ENT(elfsh_get_hashtable,
							    NULL,
							    elfsh_get_hashbucket_by_name,
							    &hashb_L2_hash,
							    elfsh_get_hashbucket_by_index,
							    NULL, NULL,
							    sizeof (int)));

  hash_add(&L1_hash, "hashchain", (void *) vm_create_L1ENT(elfsh_get_hashtable,
							   NULL,
							   elfsh_get_hashchain_by_name,
							   &hashc_L2_hash,
							   elfsh_get_hashchain_by_index,
							   NULL, NULL,
							   sizeof (int)));
}


/* Hash table for ELF header fields */
static void	setup_elfhash()
{
  hash_init(&elf_L2_hash, "elfL2objects", 29, ASPECT_TYPE_UNKNOW);
  hash_add(&elf_L2_hash, "magic"        , vm_create_L2ENT(elfsh_get_magic, 
							  elfsh_set_magic, 
							  ASPECT_TYPE_INT,
							  NULL, NULL, 
							  NULL, NULL));
							  
  hash_add(&elf_L2_hash, "class"        , vm_create_L2ENT(elfsh_get_class, 
							  elfsh_set_class, 
							  ASPECT_TYPE_INT,
							  NULL, NULL,
							  NULL, NULL));
  hash_add(&elf_L2_hash, "type"         , vm_create_L2ENT(elfsh_get_objtype, 
							  elfsh_set_objtype, 
							  ASPECT_TYPE_SHORT,
							  NULL, NULL,
							  NULL, NULL));
  hash_add(&elf_L2_hash, "machine"      , vm_create_L2ENT(elfsh_get_arch, 
							  elfsh_set_arch, 
							  ASPECT_TYPE_SHORT,
							  NULL, NULL,
							  NULL, NULL));
  hash_add(&elf_L2_hash, "version"      , vm_create_L2ENT(elfsh_get_version, 
							  elfsh_set_version, 
							  ASPECT_TYPE_INT,
							  NULL, NULL,
							  NULL, NULL));
  hash_add(&elf_L2_hash, "entry"        , vm_create_L2ENT(elfsh_get_entrypoint, 
							  elfsh_set_entrypoint, 
							  ASPECT_TYPE_CADDR,
							  NULL, NULL,
							  NULL, NULL));
  hash_add(&elf_L2_hash, "phoff"        , vm_create_L2ENT(elfsh_get_phtoff, 
							  elfsh_set_phtoff, 
							  ASPECT_TYPE_LONG,
							  NULL, NULL,
							  NULL, NULL));
  hash_add(&elf_L2_hash, "rphoff"       , vm_create_L2ENT(elfsh_get_rphtoff, 
							  elfsh_set_rphtoff, 
							  ASPECT_TYPE_LONG,
							  NULL, NULL,
							  NULL, NULL));
  hash_add(&elf_L2_hash, "shoff"        , vm_create_L2ENT(elfsh_get_shtoff, 
							  elfsh_set_shtoff, 
							  ASPECT_TYPE_LONG,
							  NULL, NULL, NULL, NULL));
  hash_add(&elf_L2_hash, "flags"        , vm_create_L2ENT(elfsh_get_flags, 
							  elfsh_set_flags, 
							  ASPECT_TYPE_INT,
							  NULL, NULL, 
							  NULL, NULL));
  hash_add(&elf_L2_hash, "ehsize"       , vm_create_L2ENT(elfsh_get_ehsize,
							  elfsh_set_ehsize, 
							  ASPECT_TYPE_SHORT,
							  NULL, NULL, 
							  NULL, NULL));
  hash_add(&elf_L2_hash, "phentsize"    , vm_create_L2ENT(elfsh_get_phentsize,
							  elfsh_set_phentsize, 
							  ASPECT_TYPE_SHORT,
							  NULL, NULL, 
							  NULL, NULL));
  hash_add(&elf_L2_hash, "shentsize"    , vm_create_L2ENT(elfsh_get_shentsize,
							  elfsh_set_shentsize, 
							  ASPECT_TYPE_SHORT,
							  NULL, NULL, 
							  NULL, NULL));
  hash_add(&elf_L2_hash, "phnum"        , vm_create_L2ENT(elfsh_get_phtnbr,
							  elfsh_set_phtnbr, 
							  ASPECT_TYPE_SHORT,
							  NULL, NULL, 
							  NULL, NULL));
  hash_add(&elf_L2_hash, "shnum"        , vm_create_L2ENT(elfsh_get_shtnbr,
							  elfsh_set_shtnbr, 
							  ASPECT_TYPE_SHORT,
							  NULL, NULL, 
							  NULL, NULL));
  hash_add(&elf_L2_hash, "shstrndx"     , vm_create_L2ENT(elfsh_get_shstrtab_index,
							  elfsh_set_shstrtab_index,
							  ASPECT_TYPE_SHORT,
							  NULL, NULL, 
							  NULL, NULL));
  hash_add(&elf_L2_hash, "pax_pageexec" , vm_create_L2ENT(elfsh_get_pax_pageexec, 
							  elfsh_set_pax_pageexec, 
							  ASPECT_TYPE_SHORT,
							  NULL, NULL, 
							  NULL, NULL));
  hash_add(&elf_L2_hash, "pax_emultramp", vm_create_L2ENT(elfsh_get_pax_emultramp, 
							  elfsh_set_pax_emultramp, 
							  ASPECT_TYPE_SHORT,
							  NULL, NULL, 
							  NULL, NULL));
  hash_add(&elf_L2_hash, "pax_mprotect" , vm_create_L2ENT(elfsh_get_pax_mprotect, 
							  elfsh_set_pax_mprotect, 
							  ASPECT_TYPE_SHORT,
							  NULL, NULL, 
							  NULL, NULL));
  hash_add(&elf_L2_hash, "pax_randmmap" , vm_create_L2ENT(elfsh_get_pax_randmmap, 
							  elfsh_set_pax_randmmap, 
							  ASPECT_TYPE_SHORT,
							  NULL, NULL, 
							  NULL, NULL));
  hash_add(&elf_L2_hash, "pax_randexec" , vm_create_L2ENT(elfsh_get_pax_randexec, 
							  elfsh_set_pax_randexec, 
							  ASPECT_TYPE_SHORT,
							  NULL, NULL, 
							  NULL, NULL));
  hash_add(&elf_L2_hash, "pax_segmexec" , vm_create_L2ENT(elfsh_get_pax_segmexec, 
							  elfsh_set_pax_segmexec, 
							  ASPECT_TYPE_SHORT,
							  NULL, NULL, 
							  NULL, NULL));
}



/* Hash table for SHT fields */
static void	setup_shthash()
{
  hash_init(&sht_L2_hash, "shtL2objects", 29, ASPECT_TYPE_UNKNOW);
  hash_add(&sht_L2_hash, "type"     , vm_create_L2ENT(elfsh_get_section_type, 
						      elfsh_set_section_type, 
						      0, NULL, NULL, NULL, NULL));
  hash_add(&sht_L2_hash, "addr"     , vm_create_L2ENT(elfsh_get_section_addr, 
						      elfsh_set_section_addr, 
						      ASPECT_TYPE_LONG, NULL, 
						      NULL, NULL, NULL));
  hash_add(&sht_L2_hash, "offset"   , vm_create_L2ENT(elfsh_get_section_foffset, 
						      elfsh_set_section_foffset, 
						      ASPECT_TYPE_LONG, NULL,
						      NULL, NULL, NULL));
  hash_add(&sht_L2_hash, "size"     , vm_create_L2ENT(elfsh_get_section_size, 
						      elfsh_set_section_size, 
						      ASPECT_TYPE_LONG, NULL,
						      NULL, NULL, NULL));
  hash_add(&sht_L2_hash, "link"     , vm_create_L2ENT(elfsh_get_section_link, 
						      elfsh_set_section_link, 
						      0, NULL, NULL, NULL, NULL));
  hash_add(&sht_L2_hash, "info"     , vm_create_L2ENT(elfsh_get_section_info, 
						      elfsh_set_section_info, 
						      0, NULL, NULL, NULL, NULL));
  hash_add(&sht_L2_hash, "align"    , vm_create_L2ENT(elfsh_get_section_align, 
						      elfsh_set_section_align, 
						      ASPECT_TYPE_LONG, NULL,
						      NULL, NULL, NULL));
  hash_add(&sht_L2_hash, "entsize"  , vm_create_L2ENT(elfsh_get_section_entsize, 
						      elfsh_set_section_entsize,
						      ASPECT_TYPE_LONG, NULL,
						      NULL, NULL, NULL));
  hash_add(&sht_L2_hash, "a"  , vm_create_L2ENT(elfsh_get_section_allocflag, 
						elfsh_set_section_allocflag, 
						ASPECT_TYPE_LONG, NULL, 
						NULL, NULL, NULL));
  hash_add(&sht_L2_hash, "w"  , vm_create_L2ENT(elfsh_get_section_writableflag, 
						elfsh_set_section_writableflag, 
						ASPECT_TYPE_LONG, NULL,
						NULL, NULL, NULL));
  hash_add(&sht_L2_hash, "x"  , vm_create_L2ENT(elfsh_get_section_execflag, 
						elfsh_set_section_execflag, 
						ASPECT_TYPE_LONG, NULL,
						NULL, NULL, NULL));
  hash_add(&sht_L2_hash, "s"  , vm_create_L2ENT(elfsh_get_section_strflag, 
						elfsh_set_section_strflag, 
						ASPECT_TYPE_LONG, NULL,
						NULL, NULL, NULL));
  hash_add(&sht_L2_hash, "m"  , vm_create_L2ENT(elfsh_get_section_mergeableflag, 
						elfsh_set_section_mergeableflag, 
						ASPECT_TYPE_LONG, NULL,
						NULL, NULL, NULL));
  hash_add(&sht_L2_hash, "l"  , vm_create_L2ENT(elfsh_get_section_linkflag, 
						elfsh_set_section_linkflag, 
						ASPECT_TYPE_LONG, NULL,
						NULL, NULL, NULL));
  hash_add(&sht_L2_hash, "o"  , vm_create_L2ENT(elfsh_get_section_orderflag, 
						elfsh_set_section_orderflag, 
						ASPECT_TYPE_LONG, NULL,
						NULL, NULL, NULL));
}

/* Hash table for PHT fields */
static void	setup_phthash()
{
  hash_init(&pht_L2_hash, "phtL2objects", 29, ASPECT_TYPE_UNKNOW);
  hash_add(&pht_L2_hash, "type"  , vm_create_L2ENT(elfsh_get_segment_type, 
						   elfsh_set_segment_type, 
						   0, NULL, NULL, NULL, NULL));
  hash_add(&pht_L2_hash, "offset", vm_create_L2ENT(elfsh_get_segment_offset, 
						   elfsh_set_segment_offset, 
						   ASPECT_TYPE_LONG, NULL,
						   NULL, NULL, NULL));
  hash_add(&pht_L2_hash, "vaddr" , vm_create_L2ENT(elfsh_get_segment_vaddr, 
						   elfsh_set_segment_vaddr, 
						   ASPECT_TYPE_LONG, NULL,
						   NULL, NULL, NULL));
  hash_add(&pht_L2_hash, "paddr" , vm_create_L2ENT(elfsh_get_segment_paddr, 
						   elfsh_set_segment_paddr, 
						   ASPECT_TYPE_LONG, NULL,
						   NULL, NULL, NULL));
  hash_add(&pht_L2_hash, "filesz", vm_create_L2ENT(elfsh_get_segment_filesz, 
						   elfsh_set_segment_filesz, 
						   ASPECT_TYPE_LONG, NULL,
						   NULL, NULL, NULL));
  hash_add(&pht_L2_hash, "memsz" , vm_create_L2ENT(elfsh_get_segment_memsz, 
						   elfsh_set_segment_memsz, 
						   ASPECT_TYPE_LONG, NULL,
						   NULL, NULL, NULL));
  hash_add(&pht_L2_hash, "flags" , vm_create_L2ENT(elfsh_get_segment_flags, 
						   elfsh_set_segment_flags, 
						   0, NULL, NULL, NULL, NULL));
  hash_add(&pht_L2_hash, "align" , vm_create_L2ENT(elfsh_get_segment_align, 
						   elfsh_set_segment_align, 
						   ASPECT_TYPE_LONG, NULL, 
						   NULL, NULL, NULL));
}


/* Hash table for symbol table */
static void	setup_symhash()
{
  hash_init(&sym_L2_hash, "symL2objects", 23, ASPECT_TYPE_UNKNOW);

  hash_add(&sym_L2_hash, "name" , vm_create_L2ENT(NULL, NULL, ASPECT_TYPE_STR, 
						  elfsh_get_symbol_name,
						  elfsh_set_symbol_name,
						  NULL, NULL));

  hash_add(&sym_L2_hash, "value", vm_create_L2ENT(elfsh_get_symbol_value, 
						  elfsh_set_symbol_value, 
						  ASPECT_TYPE_LONG, NULL,
						  NULL, NULL, NULL));
  hash_add(&sym_L2_hash, "size" , vm_create_L2ENT(elfsh_get_symbol_size, 
						  elfsh_set_symbol_size, 
						  ASPECT_TYPE_LONG, NULL,
						  NULL, NULL, NULL));
  hash_add(&sym_L2_hash, "bind" , vm_create_L2ENT(elfsh_get_symbol_bind, 
						  elfsh_set_symbol_bind, 
						  ASPECT_TYPE_BYTE, NULL, 
						  NULL, NULL, NULL));
  hash_add(&sym_L2_hash, "type" , vm_create_L2ENT(elfsh_get_symbol_type, 
						  elfsh_set_symbol_type, 
						  ASPECT_TYPE_BYTE, NULL, 
						  NULL, NULL, NULL));
  hash_add(&sym_L2_hash, "other", vm_create_L2ENT(elfsh_get_symbol_visibility, 
						  elfsh_set_symbol_visibility, 
						  ASPECT_TYPE_BYTE, NULL, 
						  NULL, NULL, NULL));

  hash_add(&sym_L2_hash, "link", vm_create_L2ENT(elfsh_get_symbol_link, 
						 elfsh_set_symbol_link, 
						 ASPECT_TYPE_SHORT, NULL, 
						 NULL, NULL, NULL));
}



/* Hash table for dynamic symbol table */
static void	setup_dynsymhash()
{
  hash_init(&dynsym_L2_hash, "dynsymL2objects", 23, ASPECT_TYPE_UNKNOW);
  hash_add(&dynsym_L2_hash, "name" , vm_create_L2ENT(NULL, NULL, ASPECT_TYPE_STR,
						     elfsh_get_dynsymbol_name,
						     elfsh_set_dynsymbol_name,
						     NULL, NULL));
  hash_add(&dynsym_L2_hash, "value", vm_create_L2ENT(elfsh_get_symbol_value, 
						     elfsh_set_symbol_value, 
						     ASPECT_TYPE_LONG, NULL,
						     NULL, NULL, NULL));
  hash_add(&dynsym_L2_hash, "size" , vm_create_L2ENT(elfsh_get_symbol_size, 
						     elfsh_set_symbol_size, 
						     ASPECT_TYPE_LONG, NULL,
						     NULL, NULL, NULL));
  hash_add(&dynsym_L2_hash, "bind" , vm_create_L2ENT(elfsh_get_symbol_bind, 
						     elfsh_set_symbol_bind, 
						     ASPECT_TYPE_BYTE, NULL, 
						     NULL, NULL, NULL));
  hash_add(&dynsym_L2_hash, "type" , vm_create_L2ENT(elfsh_get_symbol_type, 
						     elfsh_set_symbol_type, 
						     ASPECT_TYPE_BYTE, NULL, 
						     NULL, NULL, NULL));
  hash_add(&dynsym_L2_hash, "other", vm_create_L2ENT(elfsh_get_symbol_visibility, 
						     elfsh_set_symbol_visibility, 
						     ASPECT_TYPE_BYTE, NULL, 
						     NULL, NULL, NULL));
  hash_add(&dynsym_L2_hash, "link", vm_create_L2ENT(elfsh_get_symbol_link, 
						    elfsh_set_symbol_link, 
						    ASPECT_TYPE_SHORT, NULL, 
						    NULL, NULL, NULL));
}

/* Hash table for .rel sections */
static void	setup_relhash()
{
  hash_init(&rel_L2_hash, "relL2objects", 23, ASPECT_TYPE_UNKNOW);
  hash_add(&rel_L2_hash, "type"  , vm_create_L2ENT(elfsh_get_reltype, 
						   elfsh_set_reltype, 
						   ASPECT_TYPE_LONG, NULL,
						   NULL, NULL, NULL));
  hash_add(&rel_L2_hash, "sym"   , vm_create_L2ENT(elfsh_get_relsym, 
						   elfsh_set_relsym, 
						   ASPECT_TYPE_LONG, NULL,
						   NULL, NULL, NULL));
  hash_add(&rel_L2_hash, "offset", vm_create_L2ENT(elfsh_get_reloffset, 
						   elfsh_set_reloffset, 
						   ASPECT_TYPE_LONG, NULL,
						   NULL, NULL, NULL));
  hash_add(&rel_L2_hash, "addend", vm_create_L2ENT(elfsh_get_reladdend, 
						   elfsh_set_reladdend,
						   ASPECT_TYPE_LONG, NULL,
						   NULL, NULL, NULL));
}

/* Hash table for .dynamic section */
static void	setup_dynhash()
{
  hash_init(&dyn_L2_hash, "dynL2objects", 11, ASPECT_TYPE_UNKNOW);
  hash_add(&dyn_L2_hash, "val", vm_create_L2ENT(elfsh_get_dynentry_val, 
						elfsh_set_dynentry_val, 
						ASPECT_TYPE_LONG, NULL, 
						NULL, NULL, NULL));
  hash_add(&dyn_L2_hash, "tag", vm_create_L2ENT(elfsh_get_dynentry_type, 
						elfsh_set_dynentry_type, 
						ASPECT_TYPE_LONG, NULL, 
						NULL, NULL, NULL));
}



/* 
** Hash tables for GOT L2 objects : UNIMPLEMENTED for now, only 
** the value of the entry can be changed in this version of 
** the ELF shell using the got[name|idx] format without L2 field.
*/
static void	setup_gothash()
{
  
  hash_init(&got_L2_hash, "gotL2objects", 11, ASPECT_TYPE_UNKNOW);
  hash_add(&got_L2_hash, "val" , vm_create_L2ENT(elfsh_get_got_val, elfsh_set_got_val,
						 ASPECT_TYPE_LONG, NULL,
						 NULL, NULL, NULL));
  //hash_add(&got_L2_hash, "idx" , vm_create_L2ENT());
  hash_add(&got_L2_hash, "addr", vm_create_L2ENT(elfsh_get_got_addr, elfsh_set_got_addr,
						 ASPECT_TYPE_LONG, NULL,
						 NULL, NULL, NULL));
}



/* Hash tables for sections data */
static void	setup_scthash()
{
  hash_init(&sct_L2_hash, "sectsL2objects", 11, ASPECT_TYPE_UNKNOW);
  hash_add(&sct_L2_hash, "name", vm_create_L2ENT(NULL, NULL, ASPECT_TYPE_STR,
						 elfsh_get_section_name,
						 elfsh_set_section_name,
						 NULL, NULL));
  
  hash_add(&sct_L2_hash, "raw", vm_create_L2ENT(NULL, NULL, 
						ASPECT_TYPE_RAW,
						NULL, NULL, 
						elfsh_get_section_data,
						elfsh_write_section_data));
}



/* Hash table for versions sections */
static void   setup_vershash()
{
  
  /* Version symbols */
  hash_init(&vers_L2_hash, "versionL2objects", 11, ASPECT_TYPE_UNKNOW);
  hash_add(&vers_L2_hash, "value", vm_create_L2ENT(elfsh_get_versym_val,
						   elfsh_set_versym_val,
						   ASPECT_TYPE_SHORT, NULL,
						   NULL, NULL, NULL));
  
  /* Version def */
  hash_init(&verd_L2_hash, "verdefsL2objects", 11, ASPECT_TYPE_UNKNOW);
  hash_add(&verd_L2_hash, "index", vm_create_L2ENT(elfsh_get_verdef_ndx,
						   elfsh_set_verdef_ndx,
						   ASPECT_TYPE_SHORT, NULL,
						   NULL, NULL, NULL));
   hash_add(&verd_L2_hash, "count", vm_create_L2ENT(elfsh_get_verdef_cnt,
						    elfsh_set_verdef_cnt,
						    ASPECT_TYPE_SHORT, NULL,
						    NULL, NULL, NULL));
   hash_add(&verd_L2_hash, "hash", vm_create_L2ENT(elfsh_get_verdef_hash, 
						   elfsh_set_verdef_hash, 
						   ASPECT_TYPE_LONG, NULL, 
						   NULL, NULL, NULL));
   hash_add(&verd_L2_hash, "flag", vm_create_L2ENT(elfsh_get_verdef_flags, 
						   elfsh_set_verdef_flags, 
						   ASPECT_TYPE_SHORT, NULL, 
						   NULL, NULL, NULL));
   hash_add(&verd_L2_hash, "aux", vm_create_L2ENT(elfsh_get_verdef_aux, 
						  elfsh_set_verdef_aux, 
						  ASPECT_TYPE_LONG, NULL, 
						  NULL, NULL, NULL));


   /* Child & parent */
   hash_add(&verd_L2_hash, "next", vm_create_L2ENT(elfsh_get_verdef_next, 
						   elfsh_set_verdef_next, 
						   ASPECT_TYPE_LONG, NULL, 
						   NULL, NULL, NULL));
   /* Child */
   hash_add(&verd_L2_hash, "name", vm_create_L2ENT(elfsh_get_verdef_cname, 
						   elfsh_set_verdef_cname, 
						   ASPECT_TYPE_LONG, NULL, 
						   NULL, NULL, NULL));
   
   /* Version need */
   hash_init(&vern_L2_hash, "verneedL2objects", 11, ASPECT_TYPE_UNKNOW);
   hash_add(&vern_L2_hash, "index", vm_create_L2ENT(elfsh_get_verneed_ndx, 
						    elfsh_set_verneed_ndx, 
						    ASPECT_TYPE_SHORT, NULL, 
						    NULL, NULL, NULL));  
   
   hash_add(&vern_L2_hash, "name", vm_create_L2ENT(elfsh_get_verneed_name, 
						   elfsh_set_verneed_name, 
						   ASPECT_TYPE_LONG, NULL, 
						   NULL, NULL, NULL));
   
   hash_add(&vern_L2_hash, "hash", vm_create_L2ENT(elfsh_get_verneed_hash,
						   elfsh_set_verneed_hash,
						   ASPECT_TYPE_LONG, NULL,
						   NULL, NULL, NULL));
   hash_add(&vern_L2_hash, "flag", vm_create_L2ENT(elfsh_get_verneed_flags, 
						   elfsh_set_verneed_flags, 
						   ASPECT_TYPE_SHORT, NULL, 
						   NULL, NULL, NULL));
   /* child && parent */
   hash_add(&vern_L2_hash, "next", vm_create_L2ENT(elfsh_get_verneed_next, 
						   elfsh_set_verneed_next, 
						   ASPECT_TYPE_LONG, NULL, 
						   NULL, NULL, NULL));
   /* parent */
   hash_add(&vern_L2_hash, "file", vm_create_L2ENT(elfsh_get_verneed_file, 
						   elfsh_set_verneed_file, 
						   ASPECT_TYPE_LONG, NULL, 
						   NULL, NULL, NULL));
   hash_add(&vern_L2_hash, "count", vm_create_L2ENT(elfsh_get_verneed_cnt, 
						    elfsh_set_verneed_cnt, 
						    ASPECT_TYPE_SHORT, NULL, 
						    NULL, NULL, NULL));
   hash_add(&vern_L2_hash, "aux", vm_create_L2ENT(elfsh_get_verneed_aux, 
						  elfsh_set_verneed_aux, 
						  ASPECT_TYPE_LONG, NULL, 
						  NULL, NULL, NULL));
}




static void   setup_hashhash()
{

  /* Hash bucket */
  hash_init(&hashb_L2_hash, "hashbucketsL2objects", 11, ASPECT_TYPE_UNKNOW);
  hash_add(&hashb_L2_hash, "value", vm_create_L2ENT(elfsh_get_hashbucket_value, 
						    elfsh_set_hashbucket_value, 
						    ASPECT_TYPE_INT, NULL, 
						    NULL, NULL, NULL));  
  hash_add(&hashb_L2_hash, "nbucket", vm_create_L2ENT(elfsh_get_hashbucket_num, 
						      elfsh_set_hashbucket_num, 
						      ASPECT_TYPE_INT, NULL, 
						      NULL, NULL, NULL)); 
  /* Hash chain */
  hash_init(&hashc_L2_hash, "hashchainL2objects", 11, ASPECT_TYPE_UNKNOW);
  hash_add(&hashc_L2_hash, "value", vm_create_L2ENT(elfsh_get_hashchain_value, 
						    elfsh_set_hashchain_value, 
						    ASPECT_TYPE_INT, NULL, 
						    NULL, NULL, NULL));  
  hash_add(&hashc_L2_hash, "nchain", vm_create_L2ENT(elfsh_get_hashchain_num, 
						     elfsh_set_hashchain_num, 
						     ASPECT_TYPE_INT, NULL, 
						     NULL, NULL, NULL));   
}




/* Now comes Level 2 objects hash functions */
static void	setup_L2hash()
{
  setup_elfhash();
  setup_shthash();
  setup_phthash();
  setup_symhash();
  setup_relhash();
  setup_dynsymhash();
  setup_dynhash();
  setup_scthash();
  setup_gothash(); 
  setup_vershash();
  setup_hashhash();
}



/* Setup the command hash table */
static void	setup_cmdhash()
{
  hash_init(&cmd_hash, "commands", 101, ASPECT_TYPE_UNKNOW);

  /* Interactive mode / Scripting mode commands */
  if (world.state.vm_mode != ELFSH_VMSTATE_CMDLINE)
    {
      vm_addcmd(CMD_LOAD      , (void *) cmd_load     , (void *) vm_getoption   , 0, HLP_LOAD);
      vm_addcmd(CMD_UNLOAD    , (void *) cmd_unload   , (void *) vm_getoption   , 0, HLP_UNLOAD);
      vm_addcmd(CMD_SAVE      , (void *) cmd_save     , (void *) vm_getoption   , 1, HLP_SAVE);
      vm_addcmd(CMD_SWITCH    , (void *) cmd_doswitch , (void *) vm_getoption   , 0, HLP_SWITCH);
      vm_addcmd(CMD_METACMD   , (void *) cmd_meta     , (void *) NULL           , 0, HLP_METACMD);
      vm_addcmd(CMD_QUIT      , (void *) cmd_quit     , (void *) NULL           , 0, HLP_QUIT);
      vm_addcmd(CMD_QUIT2     , (void *) cmd_quit     , (void *) NULL           , 0, HLP_QUIT);
      vm_addcmd(CMD_LIST      , (void *) cmd_dolist   , (void *) NULL           , 0, HLP_LIST);
      vm_addcmd(CMD_LIST2     , (void *) cmd_dolist   , (void *) NULL           , 0, HLP_LIST);
      vm_addcmd(CMD_STOP      , (void *) cmd_stop     , (void *) NULL           , 0, HLP_STOP);
      vm_addcmd(CMD_WORKSPACE , (void *) cmd_workspace, (void *) vm_getvarparams, 0, HLP_WORKSPACE);
      vm_addcmd(CMD_WORKSPACE2, (void *) cmd_workspace, (void *) vm_getvarparams, 0, HLP_WORKSPACE);
      vm_addcmd(CMD_WORKSPACE2, (void *) cmd_workspace, (void *) vm_getvarparams, 0, HLP_WORKSPACE);
      vm_addcmd(CMD_VECTORS   , (void *) cmd_vectors  , (void *) vm_getvarparams, 0, HLP_VECTORS);
      vm_addcmd(CMD_TABLES    , (void *) cmd_tables   , (void *) vm_getvarparams, 0, HLP_TABLES);
      vm_addcmd(CMD_EMPTY     , (void *) cmd_empty    , (void *) vm_getvarparams, 0, HLP_EMPTY);
    }

  /* Command line only commands */
  if (world.state.vm_mode == ELFSH_VMSTATE_CMDLINE)
    {
      vm_addcmd(CMD_BINFILE_W, (void *) NULL, (void *) vm_getoutput, 0, "");
      vm_addcmd(CMD_BINFILE_R, (void *) NULL, (void *) vm_getinput , 0, "");
    }

  /* General purpose command */
  vm_addcmd(CMD_MODLOAD , (void *) cmd_modload  , (void *) vm_getoption   , 0, HLP_MODLOAD);
  vm_addcmd(CMD_MODULOAD, (void *) cmd_modunload, (void *) vm_getoption   , 0, HLP_MODULOAD);
  vm_addcmd(CMD_DISASM  , (void *) cmd_disasm  , (void *) vm_getdisasm    , 1, HLP_DISASM);
  vm_addcmd(CMD_DISASM2 , (void *) cmd_disasm  , (void *) vm_getdisasm    , 1, HLP_DISASM);
  vm_addcmd(CMD_HEXA    , (void *) cmd_disasm  , (void *) vm_gethexa      , 1, HLP_HEXA);
  vm_addcmd(CMD_HEXA2   , (void *) cmd_disasm  , (void *) vm_gethexa      , 1, HLP_HEXA);
  vm_addcmd(CMD_PRINT   , (void *) cmd_print   , (void *) vm_getvarparams , 0, HLP_PRINT);
  vm_addcmd(CMD_EXEC    , (void *) cmd_exec    , (void *) vm_getvarparams , 0, HLP_EXEC);
  vm_addcmd(CMD_EDIT    , (void *) cmd_edit    , (void *) vm_getoption    , 0, HLP_EDIT);
  vm_addcmd(CMD_PHT     , (void *) cmd_pht     , (void *) vm_getregxoption, 1, HLP_PHT);
  vm_addcmd(CMD_GOT     , (void *) cmd_got     , (void *) vm_getregxoption, 1, HLP_GOT);
  vm_addcmd(CMD_CTORS   , (void *) cmd_ctors   , (void *) vm_getregxoption, 1, HLP_CTORS);
  vm_addcmd(CMD_DTORS   , (void *) cmd_dtors   , (void *) vm_getregxoption, 1, HLP_DTORS);
  vm_addcmd(CMD_NOTE    , (void *) cmd_notes   , (void *) vm_getregxoption, 1, HLP_NOTES);
  vm_addcmd(CMD_CORE_INFO,(void *) cmd_core_info,(void *) NULL			  		, 1, HLP_CORE_INFO);
  vm_addcmd(CMD_SYM     , (void *) cmd_sym     , (void *) vm_getregxoption, 1, HLP_SYM);
  vm_addcmd(CMD_DYNAMIC , (void *) cmd_dyn     , (void *) vm_getregxoption, 1, HLP_DYNAMIC);
  vm_addcmd(CMD_DYNSYM  , (void *) cmd_dynsym  , (void *) vm_getregxoption, 1, HLP_DYNSYM);
  vm_addcmd(CMD_SHT     , (void *) cmd_sht     , (void *) vm_getregxoption, 1, HLP_SHT);
  vm_addcmd(CMD_DEBUG   , (void *) cmd_debug   , (void *) vm_getregxoption, 1, HLP_DEBUG);
  vm_addcmd(CMD_REL     , (void *) cmd_rel     , (void *) vm_getregxoption, 1, HLP_REL);
  vm_addcmd(CMD_COMMENT , (void *) cmd_comments, (void *) vm_getregxoption, 1, HLP_COMMENT);
  vm_addcmd(CMD_PHT2    , (void *) cmd_pht     , (void *) vm_getregxoption, 1, HLP_PHT);
  vm_addcmd(CMD_GOT2    , (void *) cmd_got     , (void *) vm_getregxoption, 1, HLP_GOT);
  vm_addcmd(CMD_CTORS2  , (void *) cmd_ctors   , (void *) vm_getregxoption, 1, HLP_CTORS);
  vm_addcmd(CMD_DTORS2  , (void *) cmd_dtors   , (void *) vm_getregxoption, 1, HLP_DTORS);
  vm_addcmd(CMD_NOTE2   , (void *) cmd_notes   , (void *) vm_getregxoption, 1, HLP_NOTES);
  vm_addcmd(CMD_SYM2    , (void *) cmd_sym     , (void *) vm_getregxoption, 1, HLP_SYM);
  vm_addcmd(CMD_DYNAMIC2, (void *) cmd_dyn     , (void *) vm_getregxoption, 1, HLP_DYNAMIC);
  vm_addcmd(CMD_DYNSYM2 , (void *) cmd_dynsym  , (void *) vm_getregxoption, 1, HLP_DYNSYM);
  vm_addcmd(CMD_SHT2    , (void *) cmd_sht     , (void *) vm_getregxoption, 1, HLP_SHT);
  vm_addcmd(CMD_REL2    , (void *) cmd_rel     , (void *) vm_getregxoption, 1, HLP_REL);
  vm_addcmd(CMD_COMMENT2, (void *) cmd_comments, (void *) vm_getregxoption, 1, HLP_COMMENT);
  vm_addcmd(CMD_SET     , (void *) cmd_set     , (void *) vm_getoption2   , 0, HLP_SET);
  vm_addcmd(CMD_ADD     , (void *) cmd_add     , (void *) vm_getoption2   , 0, HLP_ADD);
  vm_addcmd(CMD_SUB     , (void *) cmd_sub     , (void *) vm_getoption2   , 0, HLP_SUB);
  vm_addcmd(CMD_MUL     , (void *) cmd_mul     , (void *) vm_getoption2   , 0, HLP_MUL);
  vm_addcmd(CMD_DIV     , (void *) cmd_div     , (void *) vm_getoption2   , 0, HLP_DIV);
  vm_addcmd(CMD_MOD     , (void *) cmd_set     , (void *) vm_getoption2   , 0, HLP_MOD);
  vm_addcmd(CMD_INFO    , (void *) cmd_info    , (void *) NULL            , 0, HLP_INFO);
  vm_addcmd(CMD_FIXUP   , (void *) cmd_fixup   , (void *) NULL            , 1, HLP_FIXUP);
  vm_addcmd(CMD_ELF2    , (void *) cmd_elf     , (void *) NULL            , 1, HLP_ELF);
  vm_addcmd(CMD_INTERP2 , (void *) cmd_interp  , (void *) NULL            , 1, HLP_INTERP);
  vm_addcmd(CMD_ELF     , (void *) cmd_elf     , (void *) NULL            , 1, HLP_ELF);
  vm_addcmd(CMD_INTERP  , (void *) cmd_interp  , (void *) NULL            , 1, HLP_INTERP);
  vm_addcmd(CMD_HELP    , (void *) cmd_help    , (void *) vm_getvarparams , 0, HLP_HELP);
  vm_addcmd(CMD_MODHELP , (void *) cmd_modhelp , (void *) vm_getoption    , 0, HLP_MODHELP);
  vm_addcmd(CMD_SHTRM   , (void *) cmd_shtrm   , (void *) NULL            , 1, HLP_SHTRM);
  vm_addcmd(CMD_QUIET   , (void *) cmd_quiet   , (void *) NULL            , 0, HLP_QUIET);
  vm_addcmd(CMD_QUIET2  , (void *) cmd_quiet   , (void *) NULL            , 0, HLP_QUIET);
  vm_addcmd(CMD_FORCE   , (void *) cmd_force   , (void *) NULL            , 0, HLP_FORCE);
  vm_addcmd(CMD_VERB    , (void *) cmd_verb    , (void *) NULL            , 0, HLP_VERB);
  vm_addcmd(CMD_VERB2   , (void *) cmd_verb    , (void *) NULL            , 0, HLP_VERB);
  vm_addcmd(CMD_SORT    , (void *) cmd_sort    , (void *) vm_getoption    , 0, HLP_SORT);
  vm_addcmd(CMD_SORT2   , (void *) cmd_sort    , (void *) vm_getoption    , 0, HLP_SORT);
  vm_addcmd(CMD_TRACES  , (void *) cmd_traces  , (void *) vm_getvarparams , 1, HLP_TRACES);
  vm_addcmd(CMD_ALL	, (void *) cmd_glregx  , (void *) vm_getoption    , 0, HLP_ALL);
  vm_addcmd(CMD_ALL2	, (void *) cmd_glregx  , (void *) vm_getoption    , 0, HLP_ALL);
  vm_addcmd(CMD_ALERT	, (void *) cmd_alert   , (void *) vm_getoption    , 0, HLP_ALERT);
  vm_addcmd(CMD_FINDREL , (void *) cmd_findrel , (void *) NULL            , 1, HLP_FINDREL);
  vm_addcmd(CMD_WRITE	, (void *) cmd_write   , (void *) vm_getvarparams , 1, HLP_WRITE);
  vm_addcmd(CMD_APPEND  , (void *) cmd_append  , (void *) vm_getoption2   , 1, HLP_APPEND);
  vm_addcmd(CMD_EXTEND  , (void *) cmd_extend  , (void *) vm_getoption2   , 1, HLP_EXTEND);
  vm_addcmd(CMD_RELINJCT, (void *) cmd_relinject,(void *) vm_getoption2   , 0, HLP_RELINJCT);
  vm_addcmd(CMD_HIJACK  , (void *) cmd_hijack  , (void *) vm_getvarparams , 1, HLP_HIJACK);
  vm_addcmd(CMD_CMP     , (void *) cmd_cmp     , (void *) vm_getoption2   , 0, HLP_CMP);
  vm_addcmd(CMD_CMP2    , (void *) cmd_cmp     , (void *) vm_getoption2   , 0, HLP_CMP);
  vm_addcmd(CMD_INSERT  , (void *) cmd_insert  , (void *) vm_getvarparams , 1, HLP_INSERT);
  vm_addcmd(CMD_INSERT2 , (void *) cmd_insert  , (void *) vm_getvarparams , 1, HLP_INSERT);
  vm_addcmd(CMD_REMOVE  , (void *) cmd_remove  , (void *) vm_getoption2   , 1, HLP_REMOVE);
  vm_addcmd(CMD_REMOVE2 , (void *) cmd_remove  , (void *) vm_getoption2   , 1, HLP_REMOVE);
  vm_addcmd(CMD_FLUSH   , (void *) cmd_flush   , (void *) NULL            , 1, HLP_FLUSH);
  vm_addcmd(CMD_COLOR   , (void *) cmd_color   , (void *) vm_getoption3   , 0, HLP_COLOR);
  vm_addcmd(CMD_NOCOLOR , (void *) cmd_nocolor , (void *) NULL            , 0, HLP_NOCOLOR);

  vm_addcmd(CMD_JMP     , (void *) cmd_jmp     , (void *) vm_getoption    , 0, HLP_JMP);
  vm_addcmd(CMD_JE      , (void *) cmd_je      , (void *) vm_getoption    , 0, HLP_JE);
  vm_addcmd(CMD_JNE     , (void *) cmd_jne     , (void *) vm_getoption    , 0, HLP_JNE);
  vm_addcmd(CMD_JG      , (void *) cmd_jg      , (void *) vm_getoption    , 0, HLP_JG);
  vm_addcmd(CMD_JL      , (void *) cmd_jl      , (void *) vm_getoption    , 0, HLP_JL);
  vm_addcmd(CMD_JGE     , (void *) cmd_jge     , (void *) vm_getoption    , 0, HLP_JGE);
  vm_addcmd(CMD_JLE     , (void *) cmd_jle     , (void *) vm_getoption    , 0, HLP_JLE);

  vm_addcmd(CMD_FOREACH , (void *) cmd_foreach , (void *) vm_getforparams , 0, HLP_FOREACH);
  vm_addcmd(CMD_FOREND  , (void *) cmd_forend  , (void *) NULL            , 0, HLP_FOREND);
    
  vm_addcmd(CMD_SDIR    , (void *) cmd_scriptsdir, (void *) vm_getoption  , 0, HLP_SDIR);
  vm_addcmd(CMD_VLIST   , (void *) cmd_vlist   , (void *) NULL            , 0, HLP_VLIST);
  vm_addcmd(CMD_SOURCE  , (void *) cmd_source  , (void *) vm_getvarparams , 0, HLP_SOURCE);
  vm_addcmd(CMD_LSCRIPTS, (void *) cmd_lscripts, (void *) NULL		  , 0, HLP_LSCRIPTS);
  vm_addcmd(CMD_CAT	, (void *) cmd_cat     , (void *) vm_getoption    , 0, HLP_CAT);
  vm_addcmd(CMD_PROFILE	, (void *) cmd_profile , (void *) vm_getvarparams , 0, HLP_PROFILE);
  vm_addcmd(CMD_LOG     , (void *) cmd_log     , (void *) vm_getvarparams , 0, HLP_LOG);
  vm_addcmd(CMD_EXPORT  , (void *) cmd_export  , (void *) vm_getoption2   , 0, HLP_EXPORT);
  vm_addcmd(CMD_SHARED    , (void *) cmd_shared   , (void *) NULL         , 0, HLP_SHARED);
  vm_addcmd(CMD_VERSION , (void *) cmd_version , (void *) vm_getregxoption , 1, HLP_VERSION);
  vm_addcmd(CMD_VERNEED , (void *) cmd_verneed , (void *) vm_getregxoption , 1, HLP_VERNEED);
  vm_addcmd(CMD_VERDEF  , (void *) cmd_verdef  , (void *) vm_getregxoption , 1, HLP_VERDEF);
  vm_addcmd(CMD_HASH    , (void *) cmd_hashx   , (void *) vm_getregxoption , 1, HLP_HASH);
  vm_addcmd(CMD_CONFIGURE, cmd_configure       , vm_getvarparams, 0, HLP_CONFIGURE);

  /* Type related commands */
  vm_addcmd(CMD_TYPE    , cmd_type            , vm_getvarparams, 0, HLP_TYPE);
  vm_addcmd(CMD_TYPEDEF , cmd_typedef         , vm_getvarparams, 0, HLP_TYPEDEF);
  vm_addcmd(CMD_INFORM  , cmd_inform          , vm_getvarparams, 1, HLP_INFORM);
  vm_addcmd(CMD_UNINFORM, cmd_uninform        , vm_getvarparams, 0, HLP_UNINFORM);
  
#if defined(ELFSHNET)
  /* DUMP network commands */
  vm_addcmd(CMD_NETWORK   , (void *) cmd_network  , (void *) NULL            , 0, HLP_NETWORK);
  vm_addcmd(CMD_NETWORK2  , (void *) cmd_network  , (void *) NULL            , 0, HLP_NETWORK);
  vm_addcmd(CMD_NETLIST   , (void *) cmd_netlist  , (void *) NULL            , 0, HLP_NETLIST);
  vm_addcmd(CMD_NETKILL   , (void *) cmd_netkill  , (void *) vm_getoption    , 0, HLP_NETKILL);
  vm_addcmd(CMD_PEERSLIST , (void *) cmd_peerslist, (void *) NULL            , 0, HLP_PEERSLIST);
  vm_addcmd(CMD_CONNECT   , (void *) cmd_connect  , (void *) vm_getoption    , 0, HLP_CONNECT);
  vm_addcmd(CMD_DISCON    , (void *) cmd_discon   , (void *) vm_getoption    , 0, HLP_DISCON);
  vm_addcmd(CMD_RCMD	  , (void *) cmd_rcmd     , (void *) vm_getvarparams , 0, HLP_RCMD);
#endif

  /* Flow analysis commands */
  vm_addcmd(CMD_ANALYSE	  , cmd_analyse       , NULL,            1, HLP_ANALYSE);
  vm_addcmd(CMD_UNSTRIP	  , cmd_unstrip       , NULL,            1, HLP_UNSTRIP);
  vm_addcmd(CMD_GRAPH     , cmd_graph         , vm_getoption3,   1, HLP_GRAPH);
  vm_addcmd(CMD_INSPECT   , cmd_inspect       , vm_getoption,    1, HLP_INSPECT);
  vm_addcmd(CMD_FLOWJACK  , cmd_flowjack      , vm_getoption2,   1, HLP_FLOWJACK);
  vm_addcmd(CMD_ADDGOTO   , cmd_addgoto       , vm_getoption2,   1, HLP_ADDGOTO);
  vm_addcmd(CMD_SETGVL    , cmd_setgvl        , vm_getoption,    1, HLP_SETGVL);
  vm_addcmd(CMD_RENAME	  , cmd_rename        , vm_getoption2,   1, HLP_RENAME);  
  vm_addcmd(CMD_CONTROL   , cmd_control       , NULL,            1, HLP_CONTROL);
}

/* Mix default library path with LD_LIBRARY_PATH variable */
static char	*get_libpath()
{
  int		len;
  char		*ldenv;

  /* Set default value */
  strncpy(elfsh_libpath, ELFSH_LIBPATH, BUFSIZ);
  elfsh_libpath[BUFSIZ - 1] = '\0';

  len = strlen(elfsh_libpath);

  ldenv = getenv("LD_LIBRARY_PATH");

  /* Check if we use LD_LIBRARY_PATH */
  if (ldenv != NULL && len+strlen(ldenv)+2 < BUFSIZ)
    {
      /* We separate each path with ; */
      if (ldenv[0] != ';' && elfsh_libpath[len - 1] != ';')
	  strcat(elfsh_libpath, ";");
      else if (ldenv[0] == ';' && elfsh_libpath[len - 1] == ';')
	  ldenv++;

      strcat(elfsh_libpath, ldenv);
    }

  return elfsh_libpath;
}


/* Setup variables hash :
 * - Initialize $_ (last result variable) to 0 
 * - Initialize $! (last openered file) to 0
*/
static void	setup_varshash()
{
  revmobj_t	*f;
  revmobj_t	*g;
  revmobj_t	*r;
  revmobj_t	*s;
  revmobj_t	*e;
  revmobj_t	*l;
  revmobj_t	*o;

  f = vm_create_IMMED(ASPECT_TYPE_INT, 1, 0);
  g = vm_create_IMMED(ASPECT_TYPE_INT, 1, 0);
  r = vm_create_IMMED(ASPECT_TYPE_INT, 1, 0xFFFFFFFF);
  s = vm_create_IMMEDSTR(1, ELFSH_SHELL);
  e = vm_create_IMMEDSTR(1, ELFSH_EDITOR);
  l = vm_create_IMMEDSTR(1, get_libpath());
  o = vm_create_IMMED(ASPECT_TYPE_INT, 1, ELFSH_SLOG);

  hash_init(&vars_hash, "variables", 251, ASPECT_TYPE_UNKNOW);
  hash_add(&vars_hash, ELFSH_RESVAR, f);
  hash_add(&vars_hash, ELFSH_LOADVAR, g);
  hash_add(&vars_hash, ELFSH_ERRVAR, r);
  hash_add(&vars_hash, ELFSH_SHELLVAR, s);
  hash_add(&vars_hash, ELFSH_EDITVAR, e);
  hash_add(&vars_hash, ELFSH_LIBPATHVAR, l);
  hash_add(&vars_hash, ELFSH_SLOGVAR, o);
}


/* Setup ELF constants hash tables */
static void	setup_consthash()
{
  u_int		index;

  hash_init(&const_hash, "constants", 51, ASPECT_TYPE_STR);

  for (index = 0; index < ELFSH_SEGTYPE_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_seg_type[index].name, 
	     elfsh_seg_type + index);

  for (index = 0; index < ELFSH_EXTSEG_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_extseg_type[index].name, 
	     elfsh_extseg_type + index);

  for (index = 0; index < ELFSH_ENCODING_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_encoding[index].name, 
	     elfsh_encoding + index);
  for (index = 0; index < ELFSH_SHTYPE_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_sh_type[index].name, 
	     elfsh_sh_type + index);
  for (index = 0; index < ELFSH_OBJTYPE_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_obj_type[index].name, 
	     elfsh_obj_type + index);
  for (index = 0; index < ELFSH_SYMBIND_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_sym_bind[index].name, 
	     elfsh_sym_bind + index);
  for (index = 0; index < ELFSH_SYMTYPE_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_sym_type[index].name, 
	     elfsh_sym_type + index);
  for (index = 0; index < ELFSH_RELOC_i386_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_rel_type_i386[index].name, 
	     elfsh_rel_type_i386 + index);

  for (index = 0; index < ELFSH_RELOC_SPARC64_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_rel_type_sparc[index].name, 
	     elfsh_rel_type_sparc + index);

  for (index = 0; index < ELFSH_RELOC_IA64_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_rel_type_ia64[index].name, 
	     elfsh_rel_type_ia64 + index);

  for (index = 0; index < ELFSH_RELOC_ALPHA_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_rel_type_alpha[index].name, 
	     elfsh_rel_type_alpha + index);

  for (index = 0; index < ELFSH_RELOC_MIPS_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_rel_type_mips[index].name, 
	     elfsh_rel_type_mips + index);

     
  for (index = 0; index < ELFSH_DYNAMIC_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_dynentry_type[index].name, 
	     elfsh_dynentry_type + index);
  for (index = 0; index < ELFSH_EXTDYN_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_extdyn_type[index].name, 
	     elfsh_extdyn_type + index);
  for (index = 0; index < ELFSH_MIPSDYN_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_mipsdyn_type[index].name, 
	     elfsh_mipsdyn_type + index);
  for (index = 0; index < ELFSH_FEATURE_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_feature1[index].name, 
	     elfsh_feature1 + index);
  for (index = 0; index < ELFSH_POSFLAG_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_posflag1[index].name, 
	     elfsh_posflag1 + index);
  for (index = 0; index < ELFSH_FLAGS_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_flags[index].name, 
	     elfsh_flags + index);
  for (index = 0; index < ELFSH_FLAGS1_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_flags1[index].name, 
	     elfsh_flags1 + index);
  for (index = 0; index < ELFSH_MIPSFLAGS_MAX; index++)
    hash_add(&const_hash, 
	     (char *) elfsh_mipsflags[index].name, 
	     elfsh_mipsflags + index);
}


/* Setup default grammar */
/* XXX: This function is temporary and we not remain in the new type system */
void setup_grammar()
{
  hash_init(&parser_hash, "parsers", 11, ASPECT_TYPE_CADDR);

  /* Default grammar rules */
#define LOOKUP5_IDX "%41[^"REVM_SEP"]"REVM_SEP"%41[^[][%41[^]]][%41[^]]]"REVM_SEP"%41s"
#define LOOKUP4_A   "%41[^"REVM_SEP"]"REVM_SEP"%41[^[][%41[^:]:%41[^%%]%%%41[^]]]"REVM_SEP"%41s"
#define LOOKUP4_B   "%41[^"REVM_SEP"]"REVM_SEP"%41[^[][%41[^:]:%41[^]]]"REVM_SEP"%41s"
#define LOOKUP4_C   "%41[^"REVM_SEP"]"REVM_SEP"%41[^[][%41[^]]]"REVM_SEP"%41s"
#define LOOKUP3_IDX "%41[^"REVM_SEP"]"REVM_SEP"%41[^[][%41[^]]]"
#define LOOKUP3     "%41[^"REVM_SEP"]"REVM_SEP"%41[^"REVM_SEP"]"REVM_SEP"%41s"
#define LOOKUP_VECT "$vector[%41[^]]]"
#define LOOKUP_HASH "$hash[%41[^]]]"

  hash_add(&parser_hash, LOOKUP5_IDX, parse_lookup5_index);
  hash_add(&parser_hash, LOOKUP4_A, parse_lookup4);
  hash_add(&parser_hash, LOOKUP4_B, parse_lookup4);
  hash_add(&parser_hash, LOOKUP4_C, parse_lookup4);
  hash_add(&parser_hash, LOOKUP3_IDX, parse_lookup3_index);
  hash_add(&parser_hash, LOOKUP3, parse_lookup3);
  hash_add(&parser_hash, LOOKUP_VECT, parse_vector);
  hash_add(&parser_hash, LOOKUP_HASH, parse_hash);
}



/* Setup color table */
void setup_color()
{
  hash_add(&fg_color_hash, "none"   , (void *) COLOR_RESET);
  hash_add(&fg_color_hash, "black"  , (void *) COLOR_FG_BLACK);
  hash_add(&fg_color_hash, "red"    , (void *) COLOR_FG_RED);
  hash_add(&fg_color_hash, "green"  , (void *) COLOR_FG_GREEN);
  hash_add(&fg_color_hash, "yellow" , (void *) COLOR_FG_YELLOW);
  hash_add(&fg_color_hash, "blue"   , (void *) COLOR_FG_BLUE);
  hash_add(&fg_color_hash, "magenta", (void *) COLOR_FG_MAGENTA);
  hash_add(&fg_color_hash, "cyan"   , (void *) COLOR_FG_CYAN);
  hash_add(&fg_color_hash, "white"  , (void *) COLOR_FG_WHITE);

  hash_add(&bg_color_hash, "none"   , (void *) COLOR_RESET);
  hash_add(&bg_color_hash, "black"  , (void *) COLOR_BG_BLACK);
  hash_add(&bg_color_hash, "red"    , (void *) COLOR_BG_RED);
  hash_add(&bg_color_hash, "green"  , (void *) COLOR_BG_GREEN);
  hash_add(&bg_color_hash, "yellow" , (void *) COLOR_BG_YELLOW);
  hash_add(&bg_color_hash, "blue"   , (void *) COLOR_BG_BLUE);
  hash_add(&bg_color_hash, "magenta", (void *) COLOR_BG_MAGENTA);
  hash_add(&bg_color_hash, "cyan"   , (void *) COLOR_BG_CYAN);
  hash_add(&bg_color_hash, "white"  , (void *) COLOR_BG_WHITE);
}

void setup_color_type()
{
  hash_add(&t_color_hash, "string"     , (void *) vm_colorblank());
  hash_add(&t_color_hash, "fieldstring", (void *) vm_colorblank()); 
  hash_add(&t_color_hash, "typestring" , (void *) vm_colorblank());
  hash_add(&t_color_hash, "address"    , (void *) vm_colorblank());
  hash_add(&t_color_hash, "number"     , (void *) vm_colorblank());
  hash_add(&t_color_hash, "endstring"  , (void *) vm_colorblank());
  hash_add(&t_color_hash, "warnstring" , (void *) vm_colorblank());
  hash_add(&t_color_hash, "pspecial"   , (void *) vm_colorblank());
  hash_add(&t_color_hash, "psname"     , (void *) vm_colorblank());
  hash_add(&t_color_hash, "pversion"   , (void *) vm_colorblank());
  hash_add(&t_color_hash, "prelease"   , (void *) vm_colorblank());
  hash_add(&t_color_hash, "pedition"   , (void *) vm_colorblank());
  hash_add(&t_color_hash, "instr"      , (void *) vm_colorblank());
  hash_add(&t_color_hash, "function"   , (void *) vm_colorblank());
  hash_add(&t_color_hash, "filename"   , (void *) vm_colorblank());
}


/* Setup all used sub functions */
void setup_traces_table()
{
  traces_addcmd("add"    , (void *) traces_add    , 2, 1);
  traces_addcmd("rm"     , (void *) traces_rm     , 2, 1);
  traces_addcmd("enable" , (void *) traces_enable , 2, 1);
  traces_addcmd("disable", (void *) traces_disable, 2, 1);
  traces_addcmd("create" , (void *) traces_create , 2, 1);
  traces_addcmd("delete" , (void *) traces_delete , 2, 0);
  traces_addcmd("flush"  , (void *) traces_flush  , 2, 0);
}

   
/* Setup all hash tables */
void		vm_setup_hashtables()
{
  static int	done = 0;

  if (done)
    return;
  done = 1;

  hash_init(&file_hash        , "files"      , 251, ASPECT_TYPE_UNKNOW);
  hash_init(&mod_hash         , "modules"    , 51, ASPECT_TYPE_UNKNOW);
  hash_init(&labels_hash[0]   , "labels"     , 51, ASPECT_TYPE_STR);
  hash_init(&fg_color_hash    , "fgcolors"   , 13, ASPECT_TYPE_UNKNOW);
  hash_init(&bg_color_hash    , "bgcolors"   , 13, ASPECT_TYPE_UNKNOW);
  hash_init(&t_color_hash     , "tcolors"    , 11, ASPECT_TYPE_UNKNOW);
  hash_init(&traces_cmd_hash  , "traces"     , 11, ASPECT_TYPE_UNKNOW);
  hash_init(&world.shared_hash, "sharedfiles", 11, ASPECT_TYPE_UNKNOW);
  setup_varshash();
  setup_cmdhash();
  setup_consthash();
  setup_L1hash();
  setup_L2hash();
  setup_color();
  setup_color_type();
  setup_traces_table();
  setup_grammar();
  aspect_init();
}
