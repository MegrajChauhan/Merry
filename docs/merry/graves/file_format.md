# The Input File Format For Merry

The input file format for Merry is called **BEB** which stands for **Broadly Emitted Binary**. The format is quite simple really. 
If the file is to be divided into portions based on the different type of information contained, then the divisions can be made as:

- Head
- Body
- Optional Tail

Let's go through each chunk:

1) Head:
   The head is the metadata for the input file. It contains information such as the type of file, ITIT length, data section length, string section length DIT length and so on.
   I will delve into each section properly later on. 
2) Body:
   The body contains the ITIT, instructions, data and string. This is everything that is of value to the VM.
3) Tail:
   The tail is absolutely optional to have. This is the space for DIT. I will explain the importance of this section later on. It is notable that the reader completely ignores this portion and doesn't even bother reading
   it.

# Structures

## Head
The head is 40 bytes in size and occupies the first 40 bytes of the file. The structure of head goes as:

62 65 62 FT Rs Rs Rs Rs (Identification Header)
00 00 00 00 00 00 00 00 (ITIT Length in bytes)
00 00 00 00 00 00 00 00 (Data section Length in bytes)
00 00 00 00 00 00 00 00 (String section Length in bytes)
00 00 00 00 00 00 00 00 (DIT Length in bytes)

Note: FT=File Type, Rs=Reserved

The magic bytes 62, 64, and 62 in hex represent 'beb' which is used to identify the file. Currently, FT only has one value of 0 indicating a Normal input file. As reader gets more complex, it will support more file types.

| File Type | Value |
| NORMAL INPUT FILE | 0 |

It is to be noted that accept for the Identification header everything else in the file must be in little endian format.

## Body

The body is comprised of four part:

- ITIT
- Instructions
- Data Metadata
- Data
- String

### ITIT

ITIT stands for Instruction Type Identification Table. The main purpose of this table is to hint the reader on the different sections and which vcore they belong to. It is important to note that ITIT can only have at most
CORE\_COUNT entries. Here CORE\_COUNT refers to the number of types of vcores that MVM supports. This also implies that the instructions for one vcore type must not be fragmented. Each entry has a structure of:

CT Rs Rs Rs Rs Rs Rs Rs (Type Identification)
00 00 00 00 00 00 00 00 (Section Length in bytes)

Note: CT=Core Type

Section length indicates the length of the instructions section for one particular core type.

| Core Type | Value |
| GPC | 0 |

### Instructions

After ITIT is parsed, the following sections are interpreted as instructions. If the first entry to ITIT was for GPC with length 16 bytes then the reader expects the first 16 bytes following ITIT to belong to GPC.
Instructions must appear in the same order as provided in the ITIT.

### Data Metadata

Data metadata is a 24-byte data structure that provides the length of the available Quad-words(Qwords), Double-words(Dwords), and Words. The structure is something like:

00 00 00 00 00 00 00 00 (Number of Qwords * 8)
00 00 00 00 00 00 00 00 (Number of Dwords * 4)
00 00 00 00 00 00 00 00 (Number of Words * 2)

It is also to be noted that all of these numbers must be properly 8-byte, 4-byte and 2-byte aligned respectively.

### Data

Data also has a structure: 

- First the Qwords
- Followed by Dwords
- and lastly Words

Each of these sections must be as large as the respective length provided in the metadata.

### String

String section, despite the name, holds byte type values. The reason for all this separation for different data type is to make Merry cross-platform. Since everything in the file is in little endian format, Merry has to 
perform an endian conversion which means qwords can handle the conversion but the rest of the data types won't be able to handle the conversion without getting destroyed.

## Tail(Optional)

The tail is left for the user, more preciesly, assembler and compiler writers. This is the DIT section or Debug Information Table. MVM has no use for it, thus, anyone can do anything with this section. As the name suggests,
the main motive for the section is to store debugging information. Since the section is irrelevant to MVM but could be relevant to something else, the name might need a little change but yeah! Do what you feel like!

