#ifndef _MERRY_GRAVES_CONSTANTS_
#define _MERRY_GRAVES_CONSTANTS_

typedef enum mgreq_t mgreq_t; // Merry Graves Request

enum mgreq_t
{
	SHUT_DOWN,
	TRY_LOADING_NEW_PAGE_INST,
	TRY_LOADING_NEW_PAGE_DATA,
	PROBLEM_ENCOUNTERED,
	PROGRAM_REQUEST,
};

enum
{
  NONE
};

#endif
