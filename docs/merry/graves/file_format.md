# The Input File Format for Merry

The input file format for Merry is called **BEB**, which stands for **Broadly Emitted Binary**. The format is quite simple.  
If we divide the file based on the types of information it contains, the parts are:

- Head
- Body
- Optional Tail

Let’s go through each chunk:

### 1. Head

The Head is the metadata section. It contains information such as:

- File type
- ITIT length
- Data section length
- String section length
- DIT (Debug Information Table) length

I’ll go into the details of each of these fields below.

### 2. Body

The Body contains the actual content of the file. This includes:

- ITIT
- Instructions
- Data metadata
- Data
- Strings

This section holds everything valuable to the VM.

### 3. Tail (Optional)

The Tail is optional. It contains the **DIT** section, which can be used to store debug information.  
It’s important to note that the reader completely ignores this section — it’s not even read.  
This space is reserved for toolchain developers (assemblers, compilers, etc.) to store whatever additional metadata they want.

---

## Structures

### Head

The Head occupies the first **40 bytes** of the file. The layout is as follows:

62 65 62 FT Rs Rs Rs Rs (Identification Header)

00 00 00 00 00 00 00 00 (ITIT Length in bytes)

00 00 00 00 00 00 00 00 (Data Section Length in bytes)

00 00 00 00 00 00 00 00 (String Section Length in bytes)

00 00 00 00 00 00 00 00 (DIT Length in bytes)

- `FT` = File Type  
- `Rs` = Reserved  
- The first three bytes (`62 65 62`) spell "beb" in ASCII, which serves as the file's magic number.

Currently, the only defined file type is:

| File Type        | Value |
|------------------|-------|
| NORMAL INPUT FILE | 0     |

**Note:** Except for the identification header, all fields in the file must be in **little-endian** format.

---

### Body

The Body is divided into the following sections:

- **ITIT** (Instruction Type Identification Table)
- **Instructions**
- **Data Metadata**
- **Data**
- **Strings**

#### ITIT

The ITIT tells the reader how to interpret the instruction section and which vcore type each chunk of instructions belongs to.

- There can be **at most `CORE_COUNT` entries** in the ITIT.
- Each entry corresponds to a type of vcore. Instructions for the same core type **must not be fragmented**.

Each ITIT entry is 16 bytes:

CT Rs Rs Rs Rs Rs Rs Rs (Core Type Identification)

00 00 00 00 00 00 00 00 (Section Length in bytes)

- `CT` = Core Type  
- Section Length = Number of bytes of instructions for that core type.

Currently defined core types:

| Core Type | Value |
|-----------|-------|
| GPC       | 0     |

#### Instructions

The instruction section immediately follows the ITIT.  
Instructions must appear in the same order as the ITIT entries.  
For example, if the first ITIT entry is for GPC with a length of 16 bytes, the reader expects the next 16 bytes to belong to GPC instructions.

#### Data Metadata

The data metadata is 24 bytes long and describes the structure of the data section:

00 00 00 00 00 00 00 00 (Qword count * 8)

00 00 00 00 00 00 00 00 (Dword count * 4)

00 00 00 00 00 00 00 00 (Word count * 2)

- All values must be properly aligned (Qwords = 8 bytes, Dwords = 4 bytes, Words = 2 bytes).

#### Data

The Data section comes right after the metadata and is laid out in the following order:

1. Qwords
2. Dwords
3. Words

Each sub-section must match the lengths defined in the metadata.

#### String

Despite the name, this section holds raw byte data.
The reason for separating data by type is to make Merry **cross-platform**, especially for endian conversion.  
Qwords can handle conversion efficiently, but Dwords and Words might lose structure if not properly handled.

---

## Tail (Optional)

This section is reserved for **Debug Information Table (DIT)** or anything else the user wants.  
The MVM **ignores** this section entirely, meaning it has **no restrictions**.  
While originally intended for debugging metadata, the name could change to reflect more flexible usage.

Assembler and compiler developers are free to use this section however they want.

