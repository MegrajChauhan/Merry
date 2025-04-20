#ifndef _MERRY_GRAVES_INPUT_
#define _MERRY_GRAVES_INPUT_

/**
 * Format:
 * We have to consider that we have multiple core types i.e multiple ISAs to fit into a single file.
 * This is why we will make use of sections.
 * Obviously all of the cores share the same memory so data can have a single section.
 * Instructions will require separate sections for all core types or more preciesly, separate Instruction Memory for each core type
 *
 * We also need to get some information about the type of the program that is stored.
 * Since, dynamic library loading would be awesome near future, we need a flexible file format.
 *
 * The format should look something like this:
 * <IDENTIFICATION[Magic Numbers]> <File Type> <Symbol Table Available?>  -> Identification Header[We will have a few bytes left to be used for something else if it comes to that]
 * <Instruction Type Identification Table Length> -> The number of bytes that Instruction Type Identification Table will use
 * <Data Section Length> -> The number of bytes that the Data Section has
 * <String Section Length> -> Contains String[In Bytes]
 * <Symbol Table Length> -> Length of the Symbol Table[In Bytes]
 * <Symbol Identification Table Length> -> Length of Symbol Identification Table[In Bytes]
 * <Debugging Information Table Length> -> A custom section
 *
 * <ITIT>: [Core Type] [Length:8 bytes]
 * <Instruction Sections>
 * <Data Sections>
 * <String Section>
 * <Symbol Table>: [Symbols separated by '\0']
 * <Symbol Identification Table>: [Symbol Offset in ST] [Address in memory]
 * <Debugging Information>: [None of VM's business]
 *
 * A detailed explanation for each of the sections is as follow:
 * ITIT: This table provides information about all of the cores types being used. [Core Type] hints at the core that will be executing this particular section. [Length] provides the length of the section.
 *       The sections following ITIT should be the instructions exactly as specified in ITIT in the exact same order.
 * Data Section: Contains all of the data that all of the cores are going to share. Since all of the cores will be 64-bit in size despite the names given, they all share the same data memory.
 * String Section: Contains strings only. Part of the data section.
 * Symbol Table: A list of just the names of the symbols used in the program. For eg: main\0setup_server\0display_help\0run_config\0
 * SIT: Symbol Identification Table allows Merry to identify which address represents what symbol. For eg: Take the "display_help" function which is at address 0xaabbaa. Now, based on the entry in SIT(2: 0xaabbaa), Merry
 *      will know that "display_help" function was called. Since "display_help" is the third entry into ST, it's index or "offset" is 2.
 * DI: This section is a sort of "do-what-you-will" section. As Merry is planned to provide a debugging framework which may be utilized by people to write debuggers. For this cases, people may come up with conventions as 
 *     to how this section should be structured. Based on those standards and conventions, assemblers and compilers may produce the necessary information. Thus, this is a customizable section.
 *
 * It is also better to note that the contents of the file, except for the Identification Header, must be in Little Endian format. Merry will handle endian conversion if necessary.
 * */

#include "merry_types.h"
#include "merry_state.h"
#include "merry_ram.h"
#include "merry_helpers.h"
#include "merry_core_types.h"
#include "merry_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

typedef struct MerryGravesInput MerryGravesInput;
typedef enum MerryFileType MerryFileType;
typedef struct MerryInstructionTypeIdentificationTableEntry MerryITITEntry;
typedef struct MerryInstructionTypeIdentificationTable MerryITIT;
typedef struct MerryFileMetadata MerryFileMetadata;
typedef struct MerryFileSymbolTable MerryFileSymbolTable;
typedef struct MerrySymbolIdentificationTableEntry MerrySITEntry;
typedef struct MerrySymbolIdentificationTable MerrySIT;
typedef struct MerrySection MerrySection;

enum MerryFileType
{
	_NORMAL_INPUT_FILE,
};

struct MerryInstructionTypeIdentificationTableEntry
{
  mcore_t type;
  msize_t section_len;
};

struct MerryInstructionTypeIdentificationTable
{
   MerryITITEntry *entries;
   msize_t entry_count;
};

struct MerryFileMetadata
{
  MerryFileType type;
  mbool_t st_available;
  msize_t ITIT_len;
  msize_t total_instructions_len;
  msize_t data_section_len;
  msize_t string_section_len;
  msize_t ST_len;
  msize_t SIT_len;
  msize_t DI_len; // just for checksum
};

struct MerryFileSymbolTable
{
  mstr_t symbols;
  msize_t entry_count;
  msize_t file_offset;
};

struct MerrySymbolIdentificationTableEntry
{
  msize_t offset;
  maddress_t address;
  msize_t file_offset;
};

struct MerrySymbolIdentificationTable
{
  MerrySITEntry *entries;
  msize_t entry_count;
  msize_t file_offset;
};

// Here: A section is equivalent to a RAM Page.
struct MerrySection
{
  msize_t offset;
  msize_t section_length;
};

struct MerryGravesInput
{
     FILE *fd;
     mstr_t file_path;
     MerryFileMetadata metadata;
     MerryITIT itit;
     MerryFileSymbolTable sym_table;
     MerrySIT sit;
     MerrySection *instruction_offsets[__CORE_TYPE_COUNT];
     msize_t instruction_offsets_count[__CORE_TYPE_COUNT];
     MerrySection *data_offsets;
     msize_t data_offsets_count;
     MerrySection *string_offsets;
     msize_t string_offsets_count;
     MerryRAM *data_ram;
     MerryRAM *iram[__CORE_TYPE_COUNT];
     MerryState state;
     msize_t file_size;
     mbool_t _instruction_for_core_already_read[__CORE_TYPE_COUNT];
};

mbool_t merry_graves_reader_confirm_input_file(MerryGravesInput *reader);

MerryGravesInput* merry_graves_initialize_reader(mstr_t inp_path, MerryState *state);

mret_t merry_graves_reader_read_input(MerryGravesInput *reader);

void merry_graves_reader_destroy(MerryGravesInput *reader);

mret_t merry_graves_reader_parse_identification_header(MerryGravesInput * reader);

mret_t merry_graves_reader_parse_ITIT_header(MerryGravesInput *reader);

mret_t merry_graves_reader_parse_data_and_string_header(MerryGravesInput *reader);

mret_t merry_graves_reader_parse_symbol_table_header(MerryGravesInput *reader);

mret_t merry_graves_reader_parse_SIT_header(MerryGravesInput *reader);

mret_t merry_graves_reader_perform_checksum(MerryGravesInput* reader);

mret_t merry_graves_reader_parse_ITIT(MerryGravesInput *reader);

mret_t merry_graves_reader_parse_instruction_sections(MerryGravesInput *reader);

mret_t merry_graves_reader_parse_data_and_string_section(MerryGravesInput *reader);

mret_t merry_graves_reader_parse_symbol_table(MerryGravesInput *reader);

mret_t merry_graves_reader_parse_SIT(MerryGravesInput *reader);

mret_t merry_graves_reader_prep_memory(MerryGravesInput *reader);

mret_t merry_graves_reader_load_symbols(MerryGravesInput *reader);

mret_t merry_graves_reader_load_instructions(MerryGravesInput *reader, mcore_t c_type, msize_t pgnum);

mret_t merry_graves_reader_load_data(MerryGravesInput *reader, msize_t pgnum);

#endif
