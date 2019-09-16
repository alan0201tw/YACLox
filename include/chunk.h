#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum
{
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_RETURN,
} OpCode;

typedef struct
{
    int lineNumber;
    int offsetPerLine;
} LineRecord;

typedef struct
{
    int count;
    int capacity;
    LineRecord* lineRecords;
} LineRecordList;

typedef struct
{
    int count;
    int capacity;
    uint8_t* code;
    //int* lines;
    LineRecordList lineRecordList;

    // For immediate instructions (I Instructions), where values are
    // embeded in a instruction itself.
    // Where as R Instructions use address of  a register, 
    // and the actual value a instruction need is in the register.
    //
    // Reference :
    // https://en.wikibooks.org/wiki/MIPS_Assembly/Instruction_Formats
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);
void writeConstant(Chunk* chunk, Value value, int line);

#endif