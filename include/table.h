#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"

// The ratio of count to capacity is the load factor of the hash table.

typedef struct
{
    ObjString* key;
    Value value;
} Entry;

typedef struct
{
    int count;
    int capacity;
    Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(Table* table);
bool tableSet(Table* table, ObjString* key, Value value);
void tableAddAll(Table* from, Table* to);

#endif