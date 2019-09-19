#include "chunk.h"
#include "memory.h"
#include "value.h"

#include <stdio.h>

void initChunk(Chunk* chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;

    chunk->lineRecordList.count = 0;
    chunk->lineRecordList.capacity = 0;
    chunk->lineRecordList.lineRecords = NULL;
    // initialize the constants in a chunk, this will point the constants
    // to a null pointer.
    initValueArray(&(chunk->constants));
}

void freeChunk(Chunk* chunk)
{
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(LineRecord, chunk->lineRecordList.lineRecords, chunk->lineRecordList.capacity);

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
    }

    if(chunk->lineRecordList.capacity < chunk->lineRecordList.count + 1)
    {
        int oldCapacity = chunk->lineRecordList.capacity;
        chunk->lineRecordList.capacity = GROW_CAPACITY(oldCapacity);
        chunk->lineRecordList.lineRecords = GROW_ARRAY(chunk->lineRecordList.lineRecords, LineRecord, oldCapacity, chunk->lineRecordList.capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->count++;
    
    // using RLE (run-length encoding)
    // if record list is empty, put a new element in it
    // if it's not empty, compare the incoming 'line' with the last element
    // -> if they're the same, increase the count
    // -> else, put a new element in it

    if(chunk->lineRecordList.count == 0)
    {
        chunk->lineRecordList.lineRecords[0].lineNumber = line;
        chunk->lineRecordList.lineRecords[0].offsetPerLine = 0;
        chunk->lineRecordList.count++;
    }
    
    if(line == chunk->lineRecordList.lineRecords[chunk->lineRecordList.count - 1].lineNumber)
    {
        chunk->lineRecordList.lineRecords[chunk->lineRecordList.count - 1].offsetPerLine++;
    }
    else
    {
        int index = chunk->lineRecordList.count;
        chunk->lineRecordList.lineRecords[index].lineNumber = line;
        chunk->lineRecordList.lineRecords[index].offsetPerLine = 1;
        chunk->lineRecordList.count++;
    }
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

void writeConstant(Chunk* chunk, Value value, int line)
{
    int index = addConstant(chunk, value);
    
    if(index < 256)
    {
        writeChunk(chunk, OP_CONSTANT, line);
        writeChunk(chunk, index, line);
    }
    else
    {
        writeChunk(chunk, OP_CONSTANT_LONG, line);
        writeChunk(chunk, index & 0xff, line);
        writeChunk(chunk, (index >>  8) & 0xff, line);
        writeChunk(chunk, (index >> 16) & 0xff, line);
    }
}