# Overview

From this section forward, the different instructions will be described.

# Some notes before we begin:
The structure for the instructions are very simple and basic. We will define the most common structures
here and then you refer to them in the description of the instruction. Also, since we still don't have multi-byte encoding yet, the very first byte is always the opcode. 
1. No operands instructions**:
   This implies that the encoding consists of only the opcode and nothing else but zeroes.
2. Two register instructions**:
   This implies that the encoding consists of two registers as operands. In the said byte for the instruction, the upper 4 bits encode the destination register while the lower 4 bits encode the source register.
3. Immediate Register Instructions**:
   This implies that in the lower 4 bits of the said byte contains the destination register while the said bytes contain source immediate.
4. One Register Instructions**:
   This implies that the lower 4 bits of the said byte encodes one register which is the only operand.
5. One Immediate Instructions**:
   This implies the said bytes consists of an immediate which is the only operand.

**_NOTE_**: In some instructions immediate means address.
**_NOTE_**: The details and explanation here are not comprehensive so I suggest reading _docs/opcodes.txt_ for more details with examples.

# Instructions that follow type (1) encoding:
1. **nop(0x00)**: Does nothing
2. **halt(0x01)**: Halts the vcore that executes it. If only one vcore is present, the VM terminates.
3. **ret(0x29)**: Return from a procedure call
4. **pusha(0x2F)**: Push all the registers onto the stack
5. **popa(0x30)**: Pop all the registers from the stack in the opposite order of pusha.
6. **cflags(0x48)**: clear all flags.
7. **reset(0x49)**: set the value of al registers to 0
8. **clz(0x4A)**: Clear the zero flag.
9. **cln(0x4B)**: Clear the negative flag.
10. **clc(0x4C)**: Clear the carry flag.
11. **clo(0x4D)**: Clear the overflow flag.
12. **call\_excp(0xB6)**: Call a exception handler set by using set\_excp.

# Instructions that follow type (2) encoding:
## Those with last byte encoding the registers:
1. **add\_reg(0x03)**: Add the values in the two registers together
2. **sub\_reg(0x05)**: Subtract the values in the two registers together
3. **mul\_reg(0x07)**: Multiply the values in the two registers together
4. **div\_reg(0x09)**: Divide the values in the two registers together
5. **mod\_reg(0x0B)**: Perform modulus on the values in the two registers together
6. **iadd\_reg(0x0D)**: Add the values in the two registers together as signed numbers
7. **isub\_reg(0x0F)**: Subtract the values in the two registers together as signed numbers
8. **imul\_reg(0x011)**:**: Multiply the values in the two registers together as signed numbers
9. **idiv\_reg(0x13)**: Divide the values in the two registers together as signed numbers
10. **imod\_reg(0x15)**: Perform modulus on the values in the two registers together as signed numbers
11. **fadd(0x16)**: Perform floating point addition on the registers as 64-bit.
12. **fsub(0x17)**: Perform floating point subtraction on the registers as 64-bit.
13. **fmul(0x18)**: Perform floating point multiply on the registers as 64-bit.
14. **fdiv(0x19)**: Perform floating point divide on the registers as 64-bit.
15. **mov\_reg(0x1C)**: Move the value from the source into the destination register.
16. **mov\_reg8(0x1D)**:  Move just 1 byte from the source to the destination.
17. **mov\_reg16(0x1E)**: Move just 2 bytes from the source to the destination
18. **mov\_reg32(0x1F)**: Move just 4 bytes from the source to the destination
19. **movesx\_reg8(0x23)**:  Move just 1 byte from the source to the destination by sign extending it.
20. **movesx\_reg16(0x24) Move just 2 bytes from the source to the destination by sign extending it.
21. **movesx\_reg32(0x25) Move just 4 bytes from the source to the destination by sign extending it.
22. **and\_reg(0x32)**: Perform logical AND between the two value
23. **or\_reg(0x34)**: Perform logical OR between the two value
24. **xor\_reg(0x36)**: Perform logical XOR between the two value
25. **cmp\_reg(0x38)**: Compare the values in the two registers together
26. **excg8(0x41)**: Exchange 1 byte between the registers without affecting other bytes.
27. **excg16(0x42)**: Exchange 2 bytes between the registers without affecting other bytes.
28. **excg32(0x43)**: Exchange 4 bytes between the registers without affecting other bytes.
29. **excg(0x44)**: Exchange 8 bytes between the registers without affecting other bytes.
30. **mov8(0x45)**: Move 1 byte from source to destination without affecting other bytes.
31. **mov16(0x46)**: Move 2 bytes from source to destination without affecting other bytes.
32. **mov32(0x47)**: Move 4 bytes from source to destination without affecting other bytes.
33. **load\_reg(0x66)**: Load 8 bytes from memory into the destination register where the source contains the memory address
34. **store\_reg(0x67)**: Store 8 bytes into memory from the source register where the second source contains the memory address 
35. **loadb\_reg(0x68)**: Load 1 bytes from memory into the destination register where the source contains the memory address
36. **storeb\_reg(0x69)**: Store 1 bytes into memory from the source register where the second source contains the memory address 
37. **loadw\_reg(0x6A)**: Load 2 bytes from memory into the destination register where the source contains the memory address
38. **storew\_reg(0x6B)**: Store 2 bytes into memory from the source register where the second source contains the memory address 
39. **loadd\_reg(0x6C)**: Load 4 bytes from memory into the destination register where the source contains the memory address
40. **stored\_reg(0x6D)**: Store 4 bytes into memory from the source register where the second source contains the memory address 
41. **fadd32(0x89)**: Perform floating point addition on the registers as 32-bit.
42. **fsub32(0x8A)**: Perform floating point subtraction on the registers as 32-bit.
43. **fmul32(0x8B)**: Perform floating point multiply on the registers as 32-bit.
44. **fdiv32(0x8C)**: Perform floating point divide on the registers as 32-bit.

# Instructions that follow type (3) encoding:
1.  **add\_imm(0x03)**: Add the value in the register with the given immediate.
2.  **sub\_imm(0x05)**: Subtract the value in the register with the given immediate.
3.  **mul\_imm(0x07)**: Multiply the value in the register with the given immediate.
4.  **div\_imm(0x09)**: Divide the value in the register with the given immediate.
5.  **mod\_imm(0x0B)**: Perform modulus on the value in the register with the given immediate.
6.  **iadd\_imm(0x0C)**: Add the value in the register with the given immediate as signed numbers.
7.  **isub\_imm(0x0E)**: Subtract the value in the register with the given immediate as signed numbers.
8.  **imul\_imm(0x11)**: Multiply the value in the register with the given immediate as signed numbers.
9.  **idiv\_imm(0x13)**: Divide the value in the register with the given immediate as signed numbers.
10. **imod\_imm(0x14)**: Perform modulus on the value in the register with the given immediate as signed numbers.
11. **mov\_imm(0x1A)**: Move the given immediate into the register.
12. **movesx\_imm8(0x20)**:  Move the given 1 bytes immediate into the register with sign extend.
13. **movesx\_imm16(0x21)**: Move the given 2 bytes immediate into the register with sign extend.
14. **movesx\_imm32(0x22)**: Move the given 4 bytes immediate into the register with sign extend.
15. **and\_imm(0x31)**: Perform logical AND of the value in the register and the given immediate.
16. **or\_imm(0x33)**: Perform logical OR of the value in the register and the given immediate.
17. **xor\_imm(0x35)**: Perform logical XOR of the value in the register and the given immediate.
18. **cmp\_imm(0x3A)**: Compare the value in the register with the given immediate.
19. **load(0x3F)**: Load 8 bytes from the given memory address(Immediate) into the register.
20. **store(0x40)**: Store 8 bytes from the given register into the given memory address(Immediate).
21. **loadb(0x60)**: Load 1 bytes from the given memory address(Immediate) into the register.
22. **loadw(0x61)**: Load 2 bytes from the given memory address(Immediate) into the register.
23. **loadd(0x62)**: Load 4 bytes from the given memory address(Immediate) into the register.
24. **storeb(0x63)**: Store 1 bytes from the given register into the given memory address(Immediate).
25. **storew(0x64)**: Store 2 bytes from the given register into the given memory address(Immediate).
26. **stored(0x65)**: Store 4 bytes from the given register into the given memory address(Immediate).
27. **atm load(0x8D)**: atomically Load 8 bytes from the given memory address(Immediate) into the register.
28. **atm store(0x8E)**:atomically Store 8 bytes from the given register into the given memory address(Immediate).
29. **atm loadb(0x8F)**:atomically Load 1 bytes from the given memory address(Immediate) into the register.
30. **atm loadw(0x90)**:atomically Load 2 bytes from the given memory address(Immediate) into the register.
31. **atm loadd(0x91)**:atomically Load 4 bytes from the given memory address(Immediate) into the register.
32. **atm storeb(0x92)**: atomically Store 1 bytes from the given register into the given memory address(Immediate).
33. **atm storew(0x93)**: atomically Store 2 bytes from the given register into the given memory address(Immediate).
34. **atm stored(0x94)**: atomically Store 4 bytes from the given register into the given memory address(Immediate).
35. **add_memb(0x95)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
36. **add_memw(0x96)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
37. **add_memd(0x97)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
38. **add_memq(0x98)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
39. **sub_memb(0x99)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
40. **sub_memw(0x9A)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
41. **sub_memd(0x9B)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
42. **sub_memq(0x9C)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
43. **mul_memb(0x9D)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
44. **mul_memw(0x9E)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
45. **mul_memd(0x9F)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
46. **mul_memq(0xA0)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
47. **div_memb(0xA1)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
48. **div_memw(0xA2)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
49. **div_memd(0xA3)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
50. **div_memq(0xA4)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
51. **mod_memb(0xA5)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
52. **mod_memw(0xA6)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
53. **mod_memd(0xA7)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
54. **mod_memq(0xA8)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
55. **fadd_mem(0xA9)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
56. **fsub_mem(0xAA)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
57. **fmul_mem(0xAB)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
58. **fdiv_mem(0xAC)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
59. **fadd32_mem(0xAD)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
60. **fsub32_mem(0xAE)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
61. **fmul32_mem(0xAF)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
62. **fdiv32_mem(0xB0)**: Perform the said arithmetic instruction of the said length but the second operand is in the memory given by the address(Immediate).
63. **cmp_memb(0xB1)**: Perform the comparison of the said size but the second operand is in memory given by the address(Immediate).
64. **cmp_memw(0xB2)**: Perform the comparison of the said size but the second operand is in memory given by the address(Immediate).
65. **cmp_memd(0xB3)**: Perform the comparison of the said size but the second operand is in memory given by the address(Immediate).
66. **cmp_memq(0xB4)**: Perform the comparison of the said size but the second operand is in memory given by the address(Immediate).
67. **lshift(0x38)**: Perform logical left shift on the register by the given number.
68. **rshift(0x39)**: Perform logical right shift on the register by the given number.
69. **sva(0x2A); Stack Variable Access, access a variable offset by the given immediate from the BP and store into the given register.
70. **svc(0x2B)**: Stack Variable Change**: access the variable the same way but change it to what is in the register.

# Instructions that follow type (4) encoding:
1. **push\_reg(0x2D)**: Push the said register onto the stack.
2. **pop(0x2E)**: Pop a value from the stack into the said register.
4. **inc(0x3C)**: increment the said register by 1.
5. **dec(0x3D)**: decrement the said register by 1.
6. **cin(0x6F)**: Read a character into the register.
7. **cout(0x70)**: Display a character from the register.
8. **sin(0x71)**: Read a string of said length into memory.
9. **sout(0x72)**: Display a string of said lenght from memory.
10. **in(0x73)**: Read 1 bytes into the register as signed numbers.
11. **out(0x74)**: Display 1 bytes from the register as signed numbers.
12. **inw(0x75)**: Read 2 bytes into the register as signed numbers.
13. **outw(0x76)**: Display 2 bytes from the register as signed numbers.
14. **ind(0x77)**: Read 4 bytes into the register as signed numbers.
15. **outd(0x78)**: Display 4 bytes from the register as signed numbers.
16. **inq(0x79)**: Read 8 bytes into the register as signed numbers.
17. **outq(0x7A)**: Display 8 bytes from the register as signed numbers.
18. **uin(0x7B)**: Read 1 bytes into the register as unsigned numbers.
19. **uout(0x7C)**: Display 1 bytes from the register as unsigned numbers.
20. **uinw(0x7D)**: Read 2 bytes into the register as unsigned numbers.
21. **uoutw(0x7E)**: Display 2 bytes from the register as unsigned numbers.
22. **uind(0x7F)**: Read 4 bytes into the register as unsigned numbers.
23. **uoutd(0x80) Display 4 bytes from the register as unsigned numbers.
24. **uinq(0x81)**: Read 8 bytes into the register as unsigned numbers.
25. **uoutq(0x82)**: Display 8 bytes from the register as unsigned numbers.
26. **inf(0x83)**: Read 64-bit floating point number into the register
27. **outf(0x84)**: Display 64-bit floating point number from the register
28. **inf32(0x85)**: Read 32-bit floating point number into the register
29. **outf32(0x86)**: Display 32-bit floating point from number the register
30. **outr(0x87)**: Display the value of all registers as signed numbers.
31. **uoutr(0880)**: Display the value of all registers as unsigned numbers.

# Instructions that follow type (5) encoding:
1. **jmp\_off(0x26)**: Jump to the given address obtained by subtracting current PC with the offset.
2. **jmp\_addr(0x27)**: Jump to the given address given as immediate.
3. **not(0x37)**: Perform logical NOT on the given register.
4. **call(0x28)**: Jump to the said procedure by creating a stack frame.
5. **push_imm(0x2C)**: Push the given immediate onto the stack.
6. **jnz(0x4E)**: Jump to the said address if flag zero is not set.
7. **jz(0x4F)**: Jump to the said address if flag zero is set.
8. **jne(0x50)**: Jump to the said address if equal flag is not set.
9. **je(0x51)**: Jump to the said address if equal flag is set.
10. **jnc(0x52)**: Jump to the said address if carry flag is not set.
11. **jc(0x53)**: Jump to the said address if carry flag is set.
12. **jno(0x54)**: Jump to the said address if overflow flag is not set.
13. **jo(0x55)**: Jump to the said address if overflow flag is set.
14. **jnn(0x56)**: Jump to the said address if negative flag is not set.
15. **jn(0x57)**: Jump to the said address if negative flag is set.
16. **jng(0x58)**: Jump to the said address if greater flag is not set.
17. **jg(0x59)**: Jump to the said address if greater flag is set.
18. **jns(0x5A)**: Jump to the said address if small flag is not set.
19. **js(0x5B)**: Jump to the said address if small flag is set.
20. **jge(0x5C)**: Jump to the said address if equal and greater flag is set.
21. **jse(0x5D)**: Jump to the said address if equal and smaller flag is set.
22. **loop(0x5E)**: Loop to the said location until Mc register is not equal to 0.
23. **intr(0x5F)**: Send a request to the OS for some service.
24. **set\_excp(0xB5)**: Set a exception handler which will be called when call\_excp is executed.

# Instructions that do not follow any type of encoding:
1. **cmpxchg(0x6E)**: This is an atomic instruction. It's structure is**: cmpxchg \<regr1**:regr2\> \<Address\>
                     Firstly the value of regr1 is compared with what is in the given address in memory.
                     If they are equal, the value in the address is replaced by the value in regr2 but if they are no equal,
                     whatever is in regr1 is replaced by whatever is in the given address.
2. **lea(0x3E)**: Lead Effective Address. This instruction loads the effective address of something. This is very useful while
                 dealing with arrays. It's structure**: lea \<destination_regr\> \<base_regr\> \<index_regr\> \<scale_regr\>
                 This basically performs**: destination_regr = base_regr + index_regr * scale_regr
3. **move_imm_64(0x1B)**: Moves a 64-bit immediate into the said address. This instruction takes 16 bytes and the second 8 bytes
                         encode the immediate.