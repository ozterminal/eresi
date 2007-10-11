/*
** kernsh.c for libkernsh : initialisation, get_raw and mode switch
**
** $Id: kernsh.c,v 1.12 2007-09-23 17:53:35 pouik Exp $
**
*/
#include "libkernsh.h"
#include "libaspect-profiler.h"

libkernshworld_t libkernshworld;

/* Init kernsh for i386 */
int kernsh_init_i386(char *os, char *release)
{
  char buffer[256];

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  memset(&libkernshworld, '\0', sizeof(libkernshworld_t));

  libkernshworld.arch = LIBKERNSH_ARCH_I386;
  
  /* By default we use static kernel */
  kernsh_set_static_mode();
  
  config_add_item(LIBKERNSH_VMCONFIG_WITHOUT_KERNEL,
		  CONFIG_TYPE_INT,
		  CONFIG_MODE_RW,
		  (void *) 0);

  config_add_item(LIBKERNSH_VMCONFIG_USE_KERNEL,
		  CONFIG_TYPE_INT,
		  CONFIG_MODE_RW,
		  (void *) 0);

  config_add_item(LIBKERNSH_VMCONFIG_ALLOC,
		  CONFIG_TYPE_INT,
		  CONFIG_MODE_RW,
		  (void *) 0);

  config_add_item(LIBKERNSH_VMCONFIG_GZIP,
		  CONFIG_TYPE_STR,
		  CONFIG_MODE_RW,
		  (char *) LIBKERNSH_DEFAULT_GZIP);
  
  config_add_item(LIBKERNSH_VMCONFIG_OBJCOPY,
		  CONFIG_TYPE_STR,
		  CONFIG_MODE_RW,
		  (char *) LIBKERNSH_DEFAULT_OBJCOPY);

  config_add_item(LIBKERNSH_VMCONFIG_LD,
		  CONFIG_TYPE_STR,
		  CONFIG_MODE_RW,
		  (char *) LIBKERNSH_DEFAULT_LD);

  config_add_item(LIBKERNSH_VMCONFIG_FENDSIZE,
		  CONFIG_TYPE_INT,
		  CONFIG_MODE_RW,
		  (void *) LIBKERNSH_DEFAULT_FENDSIZE);

  /* We are on Linux ! */
  if (!strcmp(os, "Linux"))
    {
      if (!strncmp(release, "2.6", 3))
	{
	  libkernshworld.os = LIBKERNSH_OS_LINUX_2_6;
	}
      else if (!strncmp(release, "2.4", 3))
	{
	  libkernshworld.os = LIBKERNSH_OS_LINUX_2_4;
	}
      else 
	{
	  PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		       "Release not supported", -1);
	}
 
      config_add_item(LIBKERNSH_VMCONFIG_DEVICE,
		      CONFIG_TYPE_STR,
		      CONFIG_MODE_RW,
		      (char *) LIBKERNSH_STRING_DEVICE_MEM);
      
      config_add_item(LIBKERNSH_VMCONFIG_MODE,
		      CONFIG_TYPE_STR,
		      CONFIG_MODE_RW,
		      (char *)"write");

      config_add_item(LIBKERNSH_VMCONFIG_MMAP,
		      CONFIG_TYPE_INT,
		      CONFIG_MODE_RW,
		      (void *) 1);
      
      config_add_item(LIBKERNSH_VMCONFIG_MMAP_SIZE,
		      CONFIG_TYPE_INT,
		      CONFIG_MODE_RW,
		      (void *) LIBKERNSH_DEFAULT_LINUX_MMAP_SIZE);

      memset(buffer, '\0', sizeof(buffer));
      snprintf(buffer, sizeof(buffer), "%s-%s", LIBKERNSH_DEFAULT_LINUX_KERNEL, release);
      
      config_add_item(LIBKERNSH_VMCONFIG_KERNEL,
		      CONFIG_TYPE_STR,
		      CONFIG_MODE_RW,
		      (void *) buffer);
      
      config_add_item(LIBKERNSH_VMCONFIG_KERNELGZ,
		      CONFIG_TYPE_STR,
		      CONFIG_MODE_RW,
		      (void *) LIBKERNSH_DEFAULT_LINUX_KERNELGZ);
      
      config_add_item(LIBKERNSH_VMCONFIG_KERNELELF,
		      CONFIG_TYPE_STR,
		      CONFIG_MODE_RW,
		      (void *) LIBKERNSH_DEFAULT_LINUX_KERNELELF);

      config_add_item(LIBKERNSH_VMCONFIG_STORAGE_PATH,
		      CONFIG_TYPE_STR,
		      CONFIG_MODE_RW,
		      (void *) LIBKERNSH_DEFAULT_STORAGE_PATH);
      
      memset(buffer, '\0', sizeof(buffer));
      snprintf(buffer, sizeof(buffer), "%s-%s", LIBKERNSH_DEFAULT_LINUX_MAP, release);
  
      config_add_item(LIBKERNSH_VMCONFIG_SYSTEMMAP,
		      CONFIG_TYPE_STR,
		      CONFIG_MODE_RW,
		      (char *) buffer);

      config_add_item(LIBKERNSH_VMCONFIG_NB_SYSCALLS,
		      CONFIG_TYPE_INT,
		      CONFIG_MODE_RW,
		      (void *) LIBKERNSH_DEFAULT_LINUX_NB_SYSCALLS);

      config_add_item(LIBKERNSH_VMCONFIG_NIL_SYSCALL,
		      CONFIG_TYPE_INT,
		      CONFIG_MODE_RW,
		      (void *) LIBKERNSH_DEFAULT_LINUX_NIL_SYSCALL);

      config_add_item(LIBKERNSH_VMCONFIG_KERNEL_START,
		      CONFIG_TYPE_INT,
		      CONFIG_MODE_RW,
		      (void *) LIBKERNSH_I386_LINUX_START);
      
      config_add_item(LIBKERNSH_VMCONFIG_KERNEL_END,
		      CONFIG_TYPE_INT,
		      CONFIG_MODE_RW,
		      (void *) LIBKERNSH_I386_LINUX_END);

      config_add_item(LIBKERNSH_VMCONFIG_KLOAD,
		      CONFIG_TYPE_STR,
		      CONFIG_MODE_RW,
		      (void *) LIBKERNSH_DEFAULT_LINUX_INSMOD);

      config_add_item(LIBKERNSH_VMCONFIG_KUNLOAD,
		      CONFIG_TYPE_STR,
		      CONFIG_MODE_RW,
		      (void *) LIBKERNSH_DEFAULT_LINUX_RMMOD);

    }
  else
    {
      PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		   "Os not supported", -1);
    }
  
  XALLOC(__FILE__, 
	 __FUNCTION__,
	 __LINE__, 
	 libkernshworld.release, 
	 strlen(release) + 1, 
	 -1);
  
  if (memcpy(libkernshworld.release, release, strlen(release)) == NULL)
    {
      PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		   "Memcpy failed", -1);
    }

#if __DEBUG_KERNSH__
  printf("RELEASE %s\n", libkernshworld.release);
#endif

  kernsh_init_vectors();
  kernsh_register_vectors();

  asm_init_i386(&libkernshworld.proc);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}


int kernsh_del_i386()
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  XFREE(__FILE__, __FUNCTION__, __LINE__, libkernshworld.release);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

int kernsh_is_mem_mode()
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 
		(libkernshworld.mem == LIBKERNSH_MEM_MODE));
}

int kernsh_set_mem_mode()
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  libkernshworld.mem= LIBKERNSH_MEM_MODE;

  elfsh_set_debug_mode();

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

int kernsh_is_static_mode()
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 
		(libkernshworld.mem == LIBKERNSH_STATIC_MODE));
}

int kernsh_set_static_mode()
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  
  libkernshworld.mem= LIBKERNSH_STATIC_MODE;

  elfsh_set_static_mode();

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

int kernsh_get_mode()
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, libkernshworld.mem);
}

int kernsh_set_mode(int mode)
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  switch (mode)
    {
    case LIBKERNSH_MEM_MODE :
      libkernshworld.mem = LIBKERNSH_MEM_MODE;
      elfsh_set_debug_mode();
      break;
    case LIBKERNSH_STATIC_MODE :
      libkernshworld.mem = LIBKERNSH_STATIC_MODE;
      elfsh_set_static_mode();
      break;
    default :
      PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		   "Invalide mode", -1);
    }
  
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

/* This function is called in elfsh_get_raw to interact with the memory */
void *kernsh_elfsh_get_raw(elfshsect_t *sect)
{
  void *dataptr;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

#if __DEBUG_KERNSH__
  printf("kernsh_elfsh_get_raw\n");
#endif

  if (libkernshworld.open && kernsh_is_mem_mode() && libkernshworld.mmap)
    { 
      /* We use physical memory ? */
      if (libkernshworld.physical)
	{
#if __DEBUG_KERNSH__
	  printf("PTR 0x%lx RHDR BASE 0x%lx KERNEL_START 0x%lx SH_ADDR 0x%lx DATA 0x%lx 0x%lx\n",
		 (unsigned long) libkernshworld.ptr,
		 (unsigned long) sect->parent->rhdr.base, 
		 (unsigned long) libkernshworld.kernel_start,
		 (unsigned long) sect->shdr->sh_addr,
		 (unsigned long) sect->data,
		 (unsigned long) sect->phdr);
#endif  
	
	  dataptr = libkernshworld.ptr;
	  
	  if(elfsh_section_is_runtime(sect))
	    {
	      sect->parent->rhdr.base = 0;
	      dataptr += sect->shdr->sh_addr - libkernshworld.kernel_start;
	    }
	  else
	    {
	      sect->parent->rhdr.base = 
		libkernshworld.kernel_start - sect->shdr->sh_addr;
	    }
	
	  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, dataptr);
	}
    }
  else
    {
      if (libkernshworld.open)
	sect->parent->rhdr.base = 0;
    }

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, NULL);
}

/* This function is called in elfsh_get_raw to interact with the memory */
void *kernsh_revm_get_raw(void *addr)
{
  void *dataptr;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

#if __DEBUG_KERNSH__
  printf("kernsh_revm_get_raw\n");
#endif

  //printf("ADDR 0x%lx\n", addr);
  if (libkernshworld.open && kernsh_is_mem_mode() && libkernshworld.mmap)
    {
      /* We use physical memory ? */
      if (libkernshworld.physical)
	dataptr = libkernshworld.ptr + ((elfsh_Addr) addr - libkernshworld.kernel_start);
      else
	dataptr = libkernshworld.ptr + (elfsh_Addr) addr;

      PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, dataptr);
    }
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, NULL);
}

/* Get some information about the kernel */
int kernsh_info()
{
  int ret;
  u_int         dim[3];
  vector_t      *info;
  int          (*fct)();

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);
    
  info = aspect_vector_get(LIBKERNSH_VECTOR_NAME_INFO);
  dim[0] = libkernshworld.arch;
  dim[1] = libkernshworld.os;
  
  fct = aspect_vectors_select(info, dim);
  
  ret = fct();
  
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, ret);
}

int kernsh_info_linux()
{
  unsigned long system_call;
  char buffer[256];
  char *p;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

#if defined(__linux__)
  asm("sidt %0" : "=m" (idtr));
#endif
  
  libkernshworld.idt_base = idtr.base;
  libkernshworld.idt_limit = idtr.limit;

#if __DEBUG_KERNSH__
  printf("IDTR BASE 0x%lx LIMIT 0x%x\n", idtr.base,idtr.limit);
#endif

  kernsh_readmem(idtr.base+(2*sizeof(unsigned long))*0x80, 
		 &idt, 
		 sizeof(idt));
  system_call = (idt.off2 << 16) | idt.off1;

#if defined(__linux__)
  libkernshworld.system_call = system_call;
#endif

#if __DEBUG_KERNSH__
  printf("idt80: flags = %d flags=%X sel=%X off1=%x off2=%X\n",
	 idt.flags,(unsigned)idt.flags,(unsigned)idt.sel,(unsigned)idt.off1, 
	 (unsigned)idt.off2);
  printf("SYSTEM_CALL : 0x%lx\n", system_call);
#endif

  kernsh_readmem(system_call, buffer, 255);
  p = (char *)kernsh_find_pattern(buffer, 255, "\xff\x14\x85", 3);
      
  if (p == NULL)
    {
      PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		   "Cannot get sct", -1);
    }
  
  libkernshworld.sct = *(unsigned long *)(p + 3);

#if __DEBUG_KERNSH__
  printf("Sys Call Table 0x%lx\n", libkernshworld.sct);
#endif


#if defined(__linux__)
  asm("sgdt %0" : "=m" (gdtr));
#endif

#if __DEBUG_KERNSH__
  printf("GDTR BASE 0x%lx LIMIT 0x%x\n", gdtr.base,gdtr.limit);
#endif

  libkernshworld.gdt_base = gdtr.base;
  libkernshworld.gdt_limit = gdtr.limit;
  
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

int kernsh_info_netbsd()
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

int kernsh_info_freebsd()
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}

elfshobj_t *kernsh_load_file(char *name)
{
  elfshobj_t *ret;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  ret = elfsh_load_obj(name);
  
  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, ret);
}

void kernsh_unload_file(elfshobj_t *file)
{
  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

  elfsh_unload_obj(file);
  
  PROFILER_OUT(__FILE__, __FUNCTION__, __LINE__);
}