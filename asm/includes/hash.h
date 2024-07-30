#ifndef _HASH_
#define _HASH_

#include <stdlib.h>
#include <string.h>

#ifndef _HASH_SIZE_
#define _HASH_SIZE_ 256
#endif

// This hash map is only for strings
typedef struct hashmap hashmap;
typedef struct hashentry hashentry;

struct hashentry
{
    char *data;
    char *val;
    hashentry *next; // if two strings have the same hash
};

struct hashmap
{
    hashentry *entries;
    size_t entry_count;
};

unsigned long hash(char *str, size_t len);

hashmap *hashmap_init();

void destroy_hashmap(hashmap *map);

hashentry *hash_search(hashmap *map, char *_key);

int hash_add(hashmap *map, char *_key, char *_val);

#endif