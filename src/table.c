#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

// use parenthesis to group the constant value, just to avoid any shenanigans
#define TABLE_MAX_LOAD (0.75)

void initTable(Table* table)
{
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table* table)
{
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initTable(table);
}

static Entry* findEntry(Entry* entries, int capacity, ObjString* key)
{            
    uint32_t index = key->hash % capacity;
    for (;;)
    {
        Entry* entry = &entries[index];

        // since the hash table grows when nearly full, 
        // this can never result in an infinite loop, since there
        // will always be an empty entry.
        if (entry->key == key || entry->key == NULL)
        {
            return entry;
        }

        // linear probing
        index = (index + 1) % capacity;
    }
}

static void adjustCapacity(Table* table, int capacity)
{
    Entry* entries = ALLOCATE(Entry, capacity);

    for (size_t i = 0; i < capacity; ++i)
    {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    // re-inserting every entry in the original hash table to the new one
    for (int i = 0; i < table->capacity; i++)
    {
        Entry* entry = &table->entries[i];
        if (entry->key == NULL) continue;

        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
    }

    // free the old array / hash table
    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool tableSet(Table* table, ObjString* key, Value value)
{
    if(table->count + 1 > table->capacity * TABLE_MAX_LOAD)
    {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table->entries, table->capacity, key);

    bool isNewKey = entry->key == NULL;
    if (isNewKey) table->count++;

    entry->key = key;
    entry->value = value;
    // returns true if a new key is added
    return isNewKey;
}

void tableAddAll(Table* from, Table* to)
{   
    for (int i = 0; i < from->capacity; i++)
    {
        Entry* entry = &from->entries[i];
        if (entry->key != NULL)
        {
            tableSet(to, entry->key, entry->value);
        }
    }
}
