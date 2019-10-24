#include <stdio.h>

#include "debug.h"
#include "value.h"

static int getLine(Chunk* chunk, int offset)
{
    int offsetLeft = offset;
    for(int lineRecordIndex = 0; lineRecordIndex < chunk->lineRecordList.count; lineRecordIndex++)
    {
        if(chunk->lineRecordList.lineRecords[lineRecordIndex].offsetPerLine > offsetLeft)
        {
            return chunk->lineRecordList.lineRecords[lineRecordIndex].lineNumber;
        }
        offsetLeft -= chunk->lineRecordList.lineRecords[lineRecordIndex].offsetPerLine;
    }

    printf("Error : getLine returns -1 \n");
    return -1;
}

void disassembleChunk(Chunk* chunk, const char* name)
{
    printf("== %s ==\n", name);
    
    for(int offset = 0; offset < chunk->count;)
    {
        /*
            Instead of incrementing offset in the loop, 
            we let disassembleInstruction() do it for us.
        */
        offset = disassembleInstruction(chunk, offset);
    }
}

static int constantInstruction(const char* name, Chunk* chunk, int offset)
{
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");

    // here the +2 means the size of this instruction is 2 bytes
    // or, in other word, 2 * sizeof(uint8_t).
    // One for the opcode and one for the operand.
    //
    // PS : uint8_t is underlying by unsigned char
    return offset + 2;
}

static int constantLongInstruction(const char* name, Chunk* chunk, int offset)
{
    uint32_t constant = (chunk->code[offset + 1]) |
                        (chunk->code[offset + 2] << 8) |
                        (chunk->code[offset + 3] << 16);
                        
    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");

    return offset + 4;
}

static int simpleInstruction(const char* name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}

int disassembleInstruction(Chunk* chunk, int offset)
{
    printf("%04d ", offset);

    int lineNumber = getLine(chunk, offset);

    // if this instcution and the previous instruction shares same line number
    if(offset > 0 && lineNumber == getLine(chunk, offset - 1))
    {
        printf("   | ");
    }
    else
    {
        printf("%4d ", lineNumber);
    }

    uint8_t instruction = chunk->code[offset];
    switch (instruction)
    {
    case OP_CONSTANT:
        return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_CONSTANT_LONG:
        return constantLongInstruction("OP_CONSTANT_LONG", chunk, offset);
    
    case OP_ADD:
        return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:
        return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
        return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
        return simpleInstruction("OP_DIVIDE", offset);

    case OP_NEGATE:
      return simpleInstruction("OP_NEGATE", offset);
    case OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    
    default:
        printf("Unknown opcode %d\n", instruction);
        return offset + 1;
    }
}