/*
** $Id: libasm.h,v 1.7 2007-02-20 11:07:09 strauss Exp $
** 
** Author  : <sk at devhell dot org>
** Started : Sat Oct 26 01:18:46 2002
** Updated : Thu Apr  8 00:45:41 2004
*/

#ifndef LIBASM_H_
#define LIBASM_H_


#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __FreeBSD__
#include <sys/endian.h>
#else
#include <endian.h>
#endif

#include <libaspect.h>

#define	LIBASM_USE_VECTOR	1

/* Instruction types */
#define ASM_TYPE_NONE			0x0	   //! 
#define ASM_TYPE_IMPBRANCH	0x1    //! Branching instruction which always branch (jump).
#define ASM_TYPE_CONDBRANCH	0x2    //! Conditionnal branching instruction.
#define ASM_TYPE_CALLPROC		0x4    //! Sub Procedure calling instruction.
#define ASM_TYPE_RETPROC		0x8    //! Return instruction
#define ASM_TYPE_MEM_READ		0x10   //! Memory read access instruction.
#define ASM_TYPE_MEM_WRITE	0x20   //! Memory write access instruction.
#define ASM_TYPE_ARITH		0x40   //! Arithmetic (or logic) instruction.
#define ASM_TYPE_LOAD			0x80   //! Memory data loading instruction.
#define ASM_TYPE_STORE		0x100  //! Memory data storing instruction.
#define ASM_TYPE_ARCH			0x200  //! Architecture dependent instruction.
#define ASM_TYPE_FLAG			0x400  //! Flag-modifier instruction.
#define ASM_TYPE_INT			0x800  //! Interrupt/call-gate instruction.
#define ASM_TYPE_ASSIGN		0x1000 //! Assignment instruction.
#define ASM_TYPE_TEST			0x2000 //! Comparison or test instruction.

#define ASM_TYPE_OTHER		0x4000 //! Type that doesn't fit the ones above.


/*
  typedef's
*/

typedef struct s_asm_processor 		asm_processor;
typedef struct s_asm_instr 			asm_instr;
typedef struct s_asm_op				asm_operand;

enum 
{
  LIBASM_VECTOR_IA32,
  LIBASM_VECTOR_SPARC,
  LIBASM_VECTOR_MIPS,
  LIBASM_VECTOR_ARCHNUM
} e_vector_arch;
  
enum 
{
  ASM_PROC_IA32,
  ASM_PROC_SPARC,
  ASM_PROC_MIPS
} e_proc_type;


/*
  prototypes.
*/

/**
 * Initialize an asm_processor structure to disassemble
 * i386 instructions.
 *\param proc Pointer to an asm_processor structure
 */

void	asm_init_i386(asm_processor *);

/**
 * Initialize an asm_processor structure to disassemble
 * sparc instructions.
 *\param proc Pointer to an asm_processor structure
 */

void	asm_init_sparc(asm_processor *proc);

/**
 * Disassemble instruction 
 *
 *
 */
int	asm_read_instr(asm_instr *instr, u_char *buffer, u_int len, asm_processor *proc);

/**
 * Return instruction length.
 * 
 */

int	asm_instr_len(asm_instr *);

/**
 * Return instruction length.
 *
 */

int		asm_instruction_get_len(asm_instr *, int, void *);

/**
 * Return instruction mnemonic.
 */

char	*asm_instr_get_memonic(asm_instr *, asm_processor *);

/**
 * Return a pointer
 */

char	*asm_display_instr_att(asm_instr *, int);

void	asm_clear_instr(asm_instr *);

/**
 * Set libasm error
 */
void	asm_set_error(asm_instr *, int errorcode, char *error_msg);
int	asm_get_error_code();
char*	asm_get_error_message();

/**
 * operand field access methods.
 * use this to keep compatibility.
 **/

/**
 * Return instruction number of operands.
 *
 */

int		asm_operand_get_count(asm_instr *instr, int num, int, void *);

/**
 * Return operand content.
 */

int		asm_operand_get_content(asm_instr *, int, int, void *);

/**
 * Return operand type.
 */

int		asm_operand_get_type(asm_instr *, int, int, void *);

/**
 * Return operand size
 */

int		asm_operand_get_size(asm_instr *, int, int, void *);

int		asm_operand_get_len(asm_instr *, int, int, void *);
int		asm_operand_set_value(asm_instr *, int, int, void *);
int		asm_operand_get_value(asm_instr *, int, int, void *);

/**
 * immediate field may be a 1,2,4,8 ... byte long value
 * to set it,
 */

int		asm_operand_debug(asm_instr *, int, int, void *);

/**
 * Get immediate value stored in operand.
 */

int		asm_operand_get_immediate(asm_instr *, int, int, void *);

/**
 * Set immediate value stored in operand.
 */

int		asm_operand_set_immediate(asm_instr *, int, int, void *);
/**
 * Get base register stored in operand.
 */
int		asm_operand_get_basereg(asm_instr *, int, int, void *);
int		asm_operand_set_basereg(asm_instr *, int, int, void *);
/**
 * Get index register stored in operand (IA32 only).
 */

int		asm_operand_get_indexreg(asm_instr *, int, int, void *);

/**
 * Set index register stored in operand (IA32 only).
 */

int		asm_operand_set_indexreg(asm_instr *, int, int, void *);

/**
 * Get index scale stored in operand (IA32 only).
 */

int		asm_operand_get_scale(asm_instr *, int num, int opt, void *);

/**
 * Set index scale stored in operand (IA32 only).
 */
int		asm_operand_set_scale(asm_instr *, int num, int opt, void *);

/**
 * Set resolve handler used by asm_display_instr() to resolve immediate value.
 */
void		asm_set_resolve_handler(asm_processor *,
					void (*)(void *, u_int, char *, u_int), 
					void *);

/**
 *
 */

int asm_register_ia32_opcode(int opcode, unsigned long fcn);
int asm_register_sparc_opcode(int opcode, int opcode2, int fpop,
								unsigned long fcn);
int asm_arch_register(asm_processor *proc, int machine);
int asm_init_vectors(asm_processor *proc);

enum {
  LIBASM_ERROR_SUCCESS,
#define LIBASM_MSG_SUCCESS		"success"
  LIBASM_ERROR_NSUCH_CONTENT
#define LIBASM_MSG_NSUCH_CONTENT	"no such content"
} e_libasm_errorcode;

#include <libasm-structs.h>
#include <libasm-i386.h>
#include <libasm-sparc.h>
#include <libasm-mips.h>

#endif



