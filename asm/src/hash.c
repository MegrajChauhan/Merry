#include "hash.h"

unsigned long hash(char *str, size_t len)
{
    unsigned long h = 5381;
    for (size_t i = 0; i < len; i++)
    {
        h = ((h << 5) + h) + str[i];
    }
    return h % _HASH_SIZE_;
}

hashmap *hashmap_init()
{
    hashmap *map = (hashmap *)malloc(sizeof(hashmap));
    map->entry_count = _HASH_SIZE_;
    if (map == NULL)
        return NULL;
    map->entries = (hashentry *)malloc(sizeof(hashentry) * _HASH_SIZE_);
    for (size_t i = 0; i < map->entry_count; i++)
    {
        map->entries[i].data = NULL;
        map->entries[i].next = NULL;
    }
    if (map->entries == NULL)
    {
        free(map);
        return NULL;
    }
    return map;
}

void destroy_hashmap(hashmap *map)
{
    if (map == NULL)
        return;
    for (size_t i = 0; i < map->entry_count; i++)
    {
        if (map->entries[i].val != NULL)
        {
            free(map->entries[i].val);
            free(map->entries[i].data);
        }
        hashentry *e = map->entries[i].next;
        hashentry *temp = NULL;
        while (e != NULL)
        {
            temp = e->next;
            if (e->data != NULL)
            {
                free(e->val);
                free(e->data);
            }
            free(e);
            e = temp;
        }
    }
    free(map->entries);
    free(map);
}

hashentry *hash_search(hashmap *map, char *_key)
{
    hashentry *res = NULL;
    unsigned long _i = hash(_key, strlen(_key));

    res = &map->entries[_i];
    if (res->data == NULL)
        return NULL;
    if (strcmp(res->data, _key) == 0)
        return res;
    if (res->next != NULL)
    {
        hashentry *t = res->next;
        while (t != NULL)
        {
            if (strcmp(t->data, _key) == 0)
                return t;
            t = t->next;
        }
    }
    return NULL;
}

int hash_add(hashmap *map, char *_key, char *_val)
{
    register unsigned long len = strlen(_key);
    register unsigned long vlen = strlen(_val);
    unsigned long i = hash(_key, len);
    if (map->entries[i].data == NULL)
    {
        map->entries[i].data = (char *)malloc(len + 1);
        if (map->entries[i].data == NULL)
            return -1;
        map->entries[i].val = (char *)malloc(vlen + 1);
        if (map->entries[i].val == NULL)
        {
            free(map->entries[i].data);
            return -1;
        }
        memcpy(map->entries[i].data, _key, len + 1);
        memcpy(map->entries[i].val, _val, vlen + 1);
        return 0;
    }
    hashentry *e = &map->entries[i];
    while (e != NULL)
    {
        e = e->next;
    }
    e->next = (hashentry *)malloc(sizeof(hashentry));
    if (e->next == NULL)
        return -1;
    e->next->next = NULL;
    e->next->data = (char *)malloc(len + 1);
    if (e->next->data == NULL)
    {
        free(e->next);
        e->next = NULL;
        return -1;
    }
    e->next->val = (char *)malloc(vlen + 1);
    if (e->next->val == NULL)
    {
        free(e->next->data);
        free(e->next);
        e->next = NULL;
        return -1;
    }
    memcpy(e->next->data, _key, len + 1);
    memcpy(e->next->val, _val, vlen + 1);
    return 0;
}
