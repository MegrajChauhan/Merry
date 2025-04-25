#ifndef _MERRY_64_BIT_CORE_ISA_
#define _MERRY_64_BIT_CORE_ISA_

enum
{
  OP_NOP,  // no operation instruction
  OP_HALT, // halt instruction

  /*--------------- Arithmetic Instructions ---------------*/
  OP_ADD_IMM,
  OP_ADD_REG,
  OP_SUB_IMM,
  OP_SUB_REG,
  OP_MUL_IMM,
  OP_MUL_REG,
  OP_DIV_IMM,
  OP_DIV_REG,
  OP_MOD_IMM,
  OP_MOD_REG,

  OP_IADD_IMM,
  OP_IADD_REG,
  OP_ISUB_IMM,
  OP_ISUB_REG,
  OP_IMUL_IMM,
  OP_IMUL_REG,
  OP_IDIV_IMM,
  OP_IDIV_REG,
  OP_IMOD_IMM,
  OP_IMOD_REG,
  
  OP_FADD,
  OP_FSUB,
  OP_FMUL,
  OP_FDIV,

  OP_FADD32,
  OP_FSUB32,
  OP_FMUL32,
  OP_FDIV32,

  OP_ADD_MEMB,
  OP_ADD_MEMW,
  OP_ADD_MEMD,
  OP_ADD_MEMQ,

  OP_SUB_MEMB,
  OP_SUB_MEMW,
  OP_SUB_MEMD,
  OP_SUB_MEMQ,

  OP_MUL_MEMB,
  OP_MUL_MEMW,
  OP_MUL_MEMD,
  OP_MUL_MEMQ,

  OP_DIV_MEMB,
  OP_DIV_MEMW,
  OP_DIV_MEMD,
  OP_DIV_MEMQ,

  OP_MOD_MEMB,
  OP_MOD_MEMW,
  OP_MOD_MEMD,
  OP_MOD_MEMQ,

  OP_FADD_MEM,
  OP_FSUB_MEM,
  OP_FMUL_MEM,
  OP_FDIV_MEM,

  OP_FADD32_MEM,
  OP_FSUB32_MEM,
  OP_FMUL32_MEM,
  OP_FDIV32_MEM,

  OP_INC,
  OP_DEC,

  /* --------------------- Internal Data Movement Instructions ------------*/
  OP_MOVE_IMM_64,  // MOVE 64-bit immediate into a register 
  OP_MOVE_REG,     // move a value from one register to another
  OP_MOVE_REG8,    // move the lowest byte of one register to another
  OP_MOVE_REG16,   // move the lowest two bytes of one register to another
  OP_MOVE_REG32,   // move the lowest 4 bytes of one register to another
  OP_MOVESX_IMM8,  // move a value and sign extend it as well[only 8-bit numbers are accepted]
  OP_MOVESX_IMM16, // move a value and sign extend it as well[only 16-bit numbers are accepted]
  OP_MOVESX_IMM32, // move a value and sign extend it as well[only 32-bit numbers are accepted]
  OP_MOVESX_REG8,  // move the lower 1 byte to another register while sign extending it
  OP_MOVESX_REG16, // move the lower 2 bytes to another register while sign extending it
  OP_MOVESX_REG32, // move the lower 4 bytes to another register while sign extending it
  
  // exchanging values between two registers
  OP_EXCG8,  // exchange only 1 lower byte
  OP_EXCG16, // exchange only 2 lower bytes
  OP_EXCG32, // exchange only 4 lower bytes
  OP_EXCG,   // exchange the entire values
 
  // This also move the values but other bytes of the register is not affected
  OP_MOV8,
  OP_MOV16,
  OP_MOV32,

  // Conditional move instructions
  OP_MOVNZ,
  OP_MOVZ,
  OP_MOVNE,
  OP_MOVE,
  OP_MOVNC,
  OP_MOVC,
  OP_MOVNO,
  OP_MOVO,
  OP_MOVNN,
  OP_MOVN,
  OP_MOVNG,
  OP_MOVG,
  OP_MOVNS,
  OP_MOVS,
  OP_MOVGE,
  OP_MOVSE,

  // we won't need zero extend. It can simply be aliased with simple move
  
 /*-------------------Control Flow Instructions ------------------*/
  
  OP_JMP_OFF,  // JMP inst but the offset from current PC is provided[If the offset provided is in 2's complement then we can jump back]
  OP_JMP_ADDR, // JMP inst but the address is directly provided
  OP_JNZ,
  OP_JZ,
  OP_JNE,
  OP_JE,
  OP_JNC,
  OP_JC,
  OP_JNO,
  OP_JO,
  OP_JNN,
  OP_JN,
  OP_JNG,
  OP_JG,
  OP_JNS,
  OP_JS,
  OP_JGE,
  OP_JSE,
 
  OP_CALL,     // the call instruction

  OP_RET,      // return from a call
  OP_RETNZ,
  OP_RETZ,
  OP_RETNE,
  OP_RETE,
  OP_RETNC,
  OP_RETC,
  OP_RETNO,
  OP_RETO,
  OP_RETNN,
  OP_RETN,
  OP_RETNG,
  OP_RETG,
  OP_RETNS,
  OP_RETS,
  OP_RETGE,
  OP_RETSE,

  OP_LOOP, // automatically jumps to the given address until the specified register is 0

  OP_CALL_REG,
  OP_JMP_REGR, // address in register
	       
  OP_INTR,

  /*------------------------ Stack-Based Instructions-------------------*/

  OP_PUSH_IMM8,
  OP_PUSH_IMM16,
  OP_PUSH_IMM32,
  OP_PUSH_IMM64,
  OP_PUSH_REG,
  OP_POP8,     
  OP_POP16,     
  OP_POP32,     
  OP_POP64,     
  OP_PUSHA, // R0, R1, R2, ......
  OP_POPA,  // In reverse

  OP_PUSH_MEMB,
  OP_PUSH_MEMW,
  OP_PUSH_MEMD,
  OP_PUSH_MEMQ,
  OP_POP_MEMB,
  OP_POP_MEMW,
  OP_POP_MEMD,
  OP_POP_MEMQ,

  OP_LOADSB, // loadsb R0, 5 -> Load whatever byte is at BP+5 into R0
  OP_LOADSW,
  OP_LOADSD,
  OP_LOADSQ,

  OP_STORESB, // storesb R0, 5 -> Store the lowest byte of R0 at BP+5
  OP_STORESW,
  OP_STORESD,
  OP_STORESQ,

 /*--------------------Logical Instructions--------------------------*/

  OP_AND_IMM, // REG & IMM
  OP_AND_REG, // REG & REG

  OP_OR_IMM,  // REG | TMM
  OP_OR_REG,  // REG | REG
  
  OP_XOR_IMM, // REG ^ IMM
  OP_XOR_REG, // REG ^ REG
  
  OP_NOT,     // ~REG
  
  OP_LSHIFT,  // REG << <num>
  OP_RSHIFT,  // REG >> <num>
  OP_LSHIFT_REGR,
  OP_RSHIFT_REGR,
  
  OP_CMP_IMM, // CMP REG and IMM
  OP_CMP_REG, // CMP REG and REG
  OP_CMP_IMM_MEMB,
  OP_CMP_IMM_MEMW,
  OP_CMP_IMM_MEMD,
  OP_CMP_IMM_MEMQ,
  
  OP_FCMP,
  OP_FCMP32,

/*------------------------IO Instructions-----------------------------*/
  OP_CIN,  // character input
  OP_COUT, // character output
  OP_SIN,  // input a given number of bytes[characters or a string][string in]
  OP_SOUT, // output a given number of bytes[As characters][string out]

  OP_IN,   // input a byte
  OP_OUT,  // print a byte
  OP_INW,  // input a word[signed]
  OP_OUTW, // output a word[signed]
  OP_IND,  // input a dword[signed]
  OP_OUTD, // output a dword[signed]
  OP_INQ,  // input a qword[signed]
  OP_OUTQ, // output a qword[signed]

  OP_UIN,   // input a byte[unsigned]
  OP_UOUT,  // output a byte[unsigned]
  OP_UINW,  // input a word[unsigned]
  OP_UOUTW, // output a word[unsigned]
  OP_UIND,  // input a dword[unsigned]
  OP_UOUTD, // output a dword[unsigned]
  OP_UINQ,  // input a qword[unsigned]
  OP_UOUTQ, // output a qword[unsigned]

  OP_INF,  // read a 64-bit float
  OP_OUTF, // print a 64-bit float

  OP_INF32,  // read a 32-bit float
  OP_OUTF32, // print a 32-bit float

  OP_OUTR,  // print all register's contents as signed values[takes no operands]
  OP_UOUTR, // print all register's contents as unsigned values[takes no operands]
  OP_SIN_REG,
  OP_SOUT_REG,

/*-----------------Mem-Regr Data Movement Instructions----------------*/

  // load/store DEST/SOURCE REGR, DEST/SOURCE ADDRESS
  OP_LOADB,  // load byte
  OP_LOADW,  // load 2 bytes
  OP_LOADD,  // load 4 bytes
  OP_STOREB, // store byte
  OP_STOREW, // store 2 bytes
  OP_STORED, // store 4 bytes
  OP_LOADQ,
  OP_STOREQ,

  // the addresses are in register
  OP_LOADB_REG,
  OP_STOREB_REG,
  OP_LOADW_REG,
  OP_STOREW_REG,
  OP_LOADD_REG,
  OP_STORED_REG,
  OP_LOADQ_REG,
  OP_STOREQ_REG,

  OP_ATOMIC_LOADB,  // load byte atomically
  OP_ATOMIC_LOADW,  // load 2 bytes atomically
  OP_ATOMIC_LOADD,  // load 4 bytes atomically
  OP_ATOMIC_LOADQ,   // load 8 bytes atomically
  OP_ATOMIC_STOREB, // store byte atomically
  OP_ATOMIC_STOREW, // store 2 bytes atomically
  OP_ATOMIC_STORED, // store 4 bytes atomically
  OP_ATOMIC_STOREQ,  // store 8 bytes atomically

  OP_ATOMIC_LOADB_REG,  // load byte atomically
  OP_ATOMIC_LOADW_REG,  // load 2 bytes atomically
  OP_ATOMIC_LOADD_REG,  // load 4 bytes atomically
  OP_ATOMIC_LOADQ_REG,   // load 8 bytes atomically
  OP_ATOMIC_STOREB_REG, // store byte atomically
  OP_ATOMIC_STOREW_REG, // store 2 bytes atomically
  OP_ATOMIC_STORED_REG, // store 4 bytes atomically
  OP_ATOMIC_STOREQ_REG,  // store 8 bytes atomically

/*-------------------Utility Instructions-----------------------------*/


  /*
    LEA will require many operands.
    The first operand is the base address which can be any register.
    The second operand is the index which can also be in any register
    The third operand is the scale which can also be in any register
    The last is the destination register which can also be any register
    dest = base + index * scale [Useful for arrays: Doesn't change flags]
  */
  OP_LEA,
  
  OP_CFLAGS, // clear the flags register
  OP_RESET,  // reset all the registers

  OP_CMPXCHG, // the atomic compare and exchange instruction

  OP_CMPXCHG_REGR,
  OP_WHDLR, // set a wild handler
 };

#endif
