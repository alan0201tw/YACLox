#include "chunk.h"
#include "memory.h"
#include "value.h"

#include <stdio.h>

void initChunk(Chunk* chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    // initialize the constants in a chunk, this will point the constants
    // to a null pointer.
    initValueArray(&(chunk->constants));
}

void freeChunk(Chunk* chunk)
{
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&(chunk->constants));

    // we need to do it last
    initChunk(chunk);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line)
{
    if(chunk->capacity < chunk->count + 1)
    {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(chunk->code, uint8_t, oldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(chunk->lines, int, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

/**
 * return the index where it was appended
 */
int addConstant(Chunk* chunk, Value value)
{
    writeValueArray(&(chunk->constants), value);
    // returns the index where it was appended,
    // so that we can locate that same constant later
    return chunk->constants.count - 1;
}