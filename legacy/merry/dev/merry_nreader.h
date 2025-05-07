#ifndef _MERRY_NREADER_
#define _MERRY_NREADER_

#include <merry_utils.h>
#include <merry_types.h>
#include <merry_memory.h>
#include <merry_console.h>
#include <stdio.h>  // for FILE
#include <string.h> // for string manipulation
#include <stdlib.h> // for conversion from strings to numbers

#define _MERRY_MIN_INPFILE_LEN_ 40

#define _MERRY_STE_FLAG_ 7
#define _MERRY_DE_FLAG_ 6

#define ste_(h) (h[_MERRY_STE_FLAG_] & 1)
#define de_(h) (h[_MERRY_DE_FLAG_] & 1)
#define dfe_(h) ((h[_MERRY_DE_FLAG_] >> 1) & 1)
#define dfw_(h) ((h[_MERRY_DE_FLAG_] >> 2) & 1)

#define _MERRY_EAT_PER_ENTRY_LEN_ 8
#define _MERRY_SST_PER_ENTRY_LEN_ 16
#define _MERRY_SYMD_PER_ENTRY_LEN_ 16

#define _MERRY_GET_LITTLE_ENDIAN_(var, arr, i) \
    var = arr[i];                              \
    var <<= 8;                                 \
    var |= arr[i + 1];                         \
    var <<= 8;                                 \
    var |= arr[i + 2];                         \
    var <<= 8;                                 \
    var |= arr[i + 3];                         \
    var <<= 8;                                 \
    var |= arr[i + 4];                         \
    var <<= 8;                                 \
    var |= arr[i + 5];                         \
    var <<= 8;                                 \
    var |= arr[i + 6];                         \
    var <<= 8;                                 \
    var |= arr[i + 7];

#define rlog(msg, ...) err_log(msg, __VA_ARGS__)

typedef struct MerryReader MerryReader;
typedef struct MerryEAT MerryEAT;
typedef struct MerryInstSection MerryInstSection;
typedef struct MerrySsT MerrySsT;
typedef struct MerrySection MerrySection;
typedef enum msection_t msection_t;
typedef struct MerryST MerryST;
typedef struct MerrySymbol MerrySymbol;

struct MerrySymbol
{
    maddress_t address;
    msize_t index;
};

enum msection_t
{
    _DATA,
    _INFO,
    _SYMD,
    _OTHER,
};

struct MerryST
{
    mbptr_t st_data; /*All the entries*/
    msize_t st_len;  /*The length of ST in bytes*/
};

struct MerrySection
{
    msize_t section_len; /* Section's length in bytes */
    msection_t type;     /* The type of the section */
    msize_t st_index;    /*The index in the ST*/
    mbool_t rim, ras;    /*The flags*/
};

struct MerrySsT
{
    msize_t sst_len;               /* SsT len */
    msize_t sst_entry_count;       /*the number of entries*/
    MerrySection *sections;        /*The sections*/
    mbool_t symd_section_provided; /*This is what the VM can use for useful core dumps*/
};

struct MerryInstSection
{
    msize_t inst_section_len; /* Instruction section length */
    mqptr_t *instructions;    /* The read instructions */
    msize_t inst_page_count;  /* The instruction page count */
    msize_t start_offset;
};

struct MerryEAT
{
    maddress_t *EAT;         /* The entries of the EAT */
    msize_t eat_entry_count; /* The number of entries */
    msize_t eat_len;         /* EAT len */
};

struct MerryReader
{
    mbool_t de_flag, ste_flag, dfe_flag, dfw_flag;
    MerryInstSection inst;         /*The instruction section details*/
    MerryEAT eat;                  /* The details about the EAT section */
    MerrySsT sst;                  /*The SsT*/
    MerryST st;                    /*The ST*/
    mbptr_t *data;                 /*For data containing sections*/
    msize_t affordable_offsets[2]; /// NOTE: Doing this was more affordable then reading every page of data hence the name
    msize_t data_len;              /* The data section length in bytes */
    msize_t data_page_count;       /* How many pages are there? */
    FILE *f;                       /* the opened file */
    msize_t flen;                  /*The file's length*/
    MerrySymbol *syms;             /*Data from symd*/
    msize_t sym_count;             /*The number of symbols*/
    MerrySection *data_section;    /*Direct refrence to the data section*/
    MerrySection *str_section;     /*Direct refrence to the string section*/
    MerrySection *first;           /*The section that comes first*/
    MerrySection *second;          /*The section that comes second*/
    maddress_t overlap_st_addr;    /*The starting address of the overlapped region*/
    maddress_t overlap_ed_addr;    /*The ending address of the overlapped region*/
};

MerryReader *merry_init_reader(mcstr_t filename);

void merry_destroy_reader(MerryReader *r);

mret_t merry_reader_is_file_fit_to_read(MerryReader *r);

msize_t merry_reader_addr_to_pg_index(maddress_t addr);

mret_t merry_reader_read_header(MerryReader *r);

mret_t merry_reader_validate_header_info(MerryReader *r);

mret_t merry_reader_read_eat(MerryReader *r);

mret_t merry_reader_read_inst_page(MerryReader *r, mqptr_t store_in, msize_t pg_ind);

mret_t merry_reader_read_instructions(MerryReader *r);

mret_t merry_reader_read_sst(MerryReader *r);

mret_t merry_reader_read_data_page(MerryReader *r, msize_t address);

mret_t merry_reader_read_sections(MerryReader *r);

mret_t merry_reader_read_data_sections(MerryReader *r);

mret_t merry_reader_read_st(MerryReader *r);

mret_t merry_reader_read_file(MerryReader *r);

mbptr_t merry_reader_get_symbol(MerryReader *r, maddress_t addr);

mbool_t merry_is_valid_address(MerryReader *r, msize_t address);

mbptr_t merry_allocate_page_memory(MerryReader *r, maddress_t pg_index);

void merry_detect_multi_section(MerryReader *r, msize_t address, mbool_t *is_multi_page, mbool_t *from_first_page);

void merry_convert_to_big_endian(mbptr_t data, msize_t length);

mret_t merry_read_data(FILE *f, mbptr_t buffer, msize_t offset, msize_t length);

#endif