#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

#include <stdio.h>

int main(int argc, const char* argv[])
{
    initVM();

    Chunk chunk;
    initChunk(&chunk);
    //
    
    int constant = addConstant(&chunk, 1.22);
    writeChunk(&chunk, OP_CONSTANT, 1);
    writeChunk(&chunk, constant, 1);

    constant = addConstant(&chunk, 3.4);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);

    writeChunk(&chunk, OP_ADD, 123);

    constant = addConstant(&chunk, 5.6);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);

    writeChunk(&chunk, OP_DIVIDE, 123);

    writeChunk(&chunk, OP_NEGATE, 3);
    writeChunk(&chunk, OP_RETURN, 1);

    // writeChunk(&chunk, OP_NEGATE, 3);
    // writeChunk(&chunk, OP_RETURN, 3);

    disassembleChunk(&chunk, "test chunk");
    
    interpret(&chunk);
    freeVM();
    freeChunk(&chunk);

    return 0;
}