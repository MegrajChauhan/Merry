### The New File Format

### Flags
                      :------------------------ reserved
                      :    :------------------- reserved
                      :    :    :-------------- reserved
                      :    :    :    :--------- DE (Debugging Enabled) flag: Tells the VM to be ready for debugging.
                      :    :    :    :    :---- STE (Symbol Table Enable) flag: Indicates that the ST should be read and is available.
     0x4D 0x49 0x4E 0x00 0x00 0x00 0x00 0x00

### Sections and Sizes
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 -> The instruction section's size\
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 -> The EAT's size\
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 -> The SsT's size\
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 -> The ST's size

### EAT (Execution Address Table)
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 -> The EAT\
.\
.\
.\
.

### Instructions
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 -> The Instructions\
.\
.\
.\
.

### SsT (Sections Table)
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 -> The SsT\
.\
.\
.\
.

### Various Sections
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 -> The various sections\
.\
.\
.\
.

### ST (Symbol Table)
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 -> The ST\
.\
.\
.\
.

-------------------------------------------------------------------------------------------------------------

### Flags Description
**DE = Debug Enable flag**: Causes the VM to start looking for connections with debuggers. The VM doesn't care if the debugger connects or the connection fails. Any error during communication is ignored by the VM. For ways to communicate with the VM, refer to comms_conv.txt.

**STE = Symbol Table Enable flag**: Tells the VM that a Symbol Table exists and should be read. The Symbol Table is really useful for more detailed dump files. If the ST is not available (indicated by the ST size), the STE flag is ignored.

### EAT (Execution Address Table)
This table contains just one column with one entry. Each entry is 8 bytes in size and represents an address. Each new address tells the VM how many cores it should initialize. Each entry represents the first address for every core to start executing from. Too many cores are not recommended (Recommended: 10 at most).

### SsT (Sections Table)
The instruction section contains instructions read into memory, but what about data, strings, and sections necessary for debuggers? SsT is useful here. The SsT is also a table with 4 columns. Each row is 16 bytes in size. The structure of each entry:

Size (8 bytes) | Type (last 3 bits of the 9th byte) | Flags (10th and 11th byte and 12th byte) | Symbol Table Index (4 bytes)

**Size**: Indicates the size of the section that the entry describes. Even though 8 bytes is the limit, reading a large file is too much for the VM. Until the VM is more mature, please refrain from such inputs.

**Type**: Indicates how the section should be treated. Types include: data (Data for the program), info (For Debuggers to use), Symd (For both debuggers and VM to use; Defined by the VM), Others (For debuggers to define). Only data and Symd types are of interest to the VM. The Symd section is also a table. Each entry is 16 bytes: 8 bytes for the address of some symbol and 8 bytes for the index of that symbol in the ST. Even though ST can have no more than 2^32 - 1 bytes, this is just in case of future expansions. The data section is the same as the previous data section (see docs/input_file_format.txt for more information). The Others and Info sections are whatever the user decides. The VM ignores these two sections unless, in the case of Others, the RIM flag is set.

**Flags**:
There are no more than 2 flags for now: RIM (Read Into Memory) and RAS (Read As String). The RIM flag is only useful for the Others section. It forces the VM to read the section into memory in the same way as it reads the data section. The RAS flag, on the other hand, is very useful. If you look at how the data is read, you may question: "Then how do you read strings?" The endianness transformation inverts the strings, which is not desirable. Surely, nobody wants to see "MV yrreM". To fix this, we introduce the RAS flag, which forces the VM to read the section exactly as it is presented.

### ST (Symbol Table)
The ST is essential only for debugging. It is indispensable for proper debugging and making sense of nonsensical hexadecimal numbers. Here is an example of the ST table (I am using ASCII characters, but they are in binary of course):

main\0add_two_numbers\0start_thread\0end_of_world!!do_something!!\0we_are_in_the_symbol_table\0

This is a short and simple example of ST. This ST describes 5 functions. Say the program crashed while executing the we_are_in_the_symbol_table procedure. The VM knows the address of the procedure, say it is 0xFF44532. The VM also reads the Symd section. What does it do now? The VM looks for that address in the ordered pairs it has, finds 0xFF44532, and also finds the ST index, which happens to be 67 (counting could have been wrong, starts from 0). Now the VM goes to index 67 in the ST and finds the letter 'w' there. Starting from there, the VM reads until the terminating character and figures out the name of the procedure. If the '--dump-files' or '-df' option was provided, the VM will produce a dump file and backtrace the function calls. This is when the ST becomes useful. If the provided index is wrong or ST isn't available, "NO SYMBOL FOUND" is printed instead.

Refer to abc0 and abc1 files for examples of what the dump files look like.

## New Header Flags added:
Apart from the DE flag and STE flag, we have two new flags: DFE flag and DFW flag.

**DFE flag:** DFE\(Debug Follow Enable\) tells the VM that any new process should also look for debugger connections. For now we have to think about solving the different port problem with the new process which will be solved soon enough.
Position: second last bit of the second last byte.
**DFW flag:** DFW\(Debug Follow Wait) tells what the **--freeze, -f** options do to the new processes. 
Position: third last bit of the second last byte.

# Flags and Type representations as well as Positions
**DFE flag:** The 2nd last bit of the 7th byte of the header.
**DFW flag:** The 3nd last bit of the 7th byte of the header.
**Data:** If the section is data then the 3 bits = 0. 
**Info:** If the section is info then the 3 bits = 1. 
**Symd:** If the section is symd then the 3 bits = 2. 
**Other:** If the section is other then the 3 bits = 3.
**RIM:** Last bit of the 9th byte of the respective SsT entry 
**RAS:** Last bit of the 10th byte of the respective SsT entry 

**Note:** The purpose of the ST index in the entry is to give the section a name. This can be used by the assembler to pass specific named sections.