#ifndef _MERRY_NREADER_
#define _MERRY_NREADER_

#if defined(_WIN64)
#include "..\..\utils\merry_utils.h"
#else
#include "../../utils/merry_utils.h"
#endif

#include "merry_memory.h"
#include <stdio.h>  // for FILE
#include <string.h> // for string manipulation
#include <stdlib.h> // for conversion from strings to numbers

#define _MERRY_MIN_INPFILE_LEN_ 40

#define _MERRY_STE_FLAG_ 7
#define _MERRY_DE_FLAG_ 6

#define _MERRY_EAT_PER_ENTRY_LEN_ 8
#define _MERRY_SST_PER_ENTRY_LEN_ 16

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

#define rlog(msg, ...) fprintf(msg, __VA_ARGS__)

typedef struct MerryReader MerryReader;
typedef struct MerryEAT MerryEAT;
typedef struct MerryInstSection MerryInstSection;
typedef struct MerrySsT MerrySsT;
typedef struct MerrySection MerrySection;
typedef enum msection_t msection_t;
typedef struct MerryST MerryST;

enum msection_t
{
    _DATA,
    _INFO,
    _SYMD,
    _OTHER,
};

struct MerryST
{
    mstr_t *st_entries;     /*All the entries*/
    msize_t st_entry_count; /*The number of entries*/
    msize_t st_len;         /*The length of ST in bytes*/
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
};

struct MerryEAT
{
    maddress_t *EAT;         /* The entries of the EAT */
    msize_t eat_entry_count; /* The number of entries */
    msize_t eat_len;         /* EAT len */
};

struct MerryReader
{
    mbool_t de_flag, ste_flag;
    MerryInstSection inst;   /*The instruction section details*/
    MerryEAT eat;            /* The details about the EAT section */
    MerrySsT sst;            /*The SsT*/
    MerryST st;              /*The ST*/
    mbptr_t *data;           /*For data containing sections*/
    msize_t data_len;        /* The data section length in bytes */
    msize_t data_page_count; /* How many pages are there? */
    FILE *f;                 /* the opened file */
    msize_t flen;            /*The file's length*/
};

MerryReader *merry_init_reader(mcstr_t filename);

void merry_destroy_reader(MerryReader *r);

mret_t merry_reader_is_file_fit_to_read(MerryReader *r);

mret_t merry_reader_read_arr(MerryReader *r);

mret_t merry_reader_validate_arr_ifo(MerryReader *r);

mret_t merry_reader_read_eat(MerryReader *r);

mret_t merry_reader_read_instructions(MerryReader *r);

mret_t merry_reader_read_sst(MerryReader *r);

#endif