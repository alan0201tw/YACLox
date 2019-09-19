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
    writeChunk(&chunk, OP_RETURN, 0);
    writeConstant(&chunk, 1.1, 1);
    writeChunk(&chunk, OP_RETURN, 2);
    writeConstant(&chunk, 2.3, 3);
    writeChunk(&chunk, OP_RETURN, 3);

    writeConstant(&chunk, 5.7, 8);
    writeConstant(&chunk, 5.7, 8);
    writeConstant(&chunk, 5.7, 8);
    int tmp = 312;
    while(tmp--)
        writeConstant(&chunk, tmp, 315 - tmp);

    disassembleChunk(&chunk, "test chunk");

    // for(int lineRecordIndex = 0; lineRecordIndex < chunk.lineRecordList.count; lineRecordIndex++)
    // {
    //     printf("line record[%d] : lineNumber = %d, instructionPerLine = %d \n", 
    //         lineRecordIndex, 
    //         chunk.lineRecordList.lineRecords[lineRecordIndex].lineNumber,
    //         chunk.lineRecordList.lineRecords[lineRecordIndex].offsetPerLine);
    // }
    
    interpret(&chunk);
    freeVM();
    freeChunk(&chunk);

    return 0;
}