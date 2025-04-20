#ifndef _MERRY_CORE_TYPES_
#define _MERRY_CORE_TYPES_

typedef enum mcore_t mcore_t;

enum mcore_t
{
	__CORE_8_BIT,
	__CORE_16_BIT,
	__CORE_32_BIT,
	__CORE_64_BIT,

	// We can have many more types here
        __CORE_TYPE_COUNT,
};

#endif
