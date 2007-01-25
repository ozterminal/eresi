/**
 * $Id: libasm-mips-structs.h,v 1.1 2007-01-23 09:53:03 heroine Exp $
 *
 *
 */
#ifndef LIBASM_MIPS_STRUCTS_H
#define LIBASM_MIPS_STRUCTS_H

/**
 * Structure used to decode format reg instructions
 */
struct s_mips_decode_reg
{
  u_int32_t	op:6;	/*! opcode field		*/
  u_int32_t	rs:5;	/*! 1st source register		*/
  u_int32_t	rt:5;	/*! 2nd source register		*/
  u_int32_t	rd:5;	/*! destination register	*/
  u_int32_t	sa:5;	/*! shift amount (shift instr)	*/
  u_int32_t	fn:6;	/*! function to perform		*/
};

/**
 * Structure used to decode format immediate instructions
 */
struct s_mips_decode_imm
{
  u_int32_t	op:6;
  u_int32_t	rs:5;
  u_int32_t	rt:5;
  u_int32_t	im:16;
};

/**
 * Structure used to decode format jump instructions
 */
struct s_mips_decode_jump
{
  u_int32_t	op:6;	/*! opcode field		*/
  u_int32_t	ta:26;	/*! target to jump to		*/
};

#endif