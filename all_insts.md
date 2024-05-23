# Overview

From this section forward, the different instructions will be described.
Each instruction will follow the pattern:
opcode(o)
structure(s):
example(e) (if deemed necessary)
details(d)(if necessary or left to reader's intuition)

boyc = bytes of your choice
boi = bytes of immediate

1. nop
	1. o: 0x00
	2. s: 0x00 <6 boyc>
	3. d: does nothing
2. halt
	1. o: 0x01
	2. s: 0x01
	3. d: halts the core and exits VM if only one core is running
3. add_imm
	1. o: 0x02
	2. s: 0x02 <1 byte: lower 4 bits = destination register> <6 boi>
	3. d: Adds the immediate to the register
	4. e: 0x02 0x01 0x00 0xAA 0XBB 0xCC 0xDD 0xEE 
4. add_reg
	1. o: 0x03
	2. s: 0x03 <6 boyc> <1 byte: upper 4 bits = destination register, lower 4 bits = source register>
	3. d: Adds the value in the second register to the first register
	4. e: 0x03 0x00 0x00 0x00 0x00 0x00 0x00 0x12 
