#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

void compile(const char* source)
{
    initScanner(source);

    int line = -1;
    for(;;)
    {
        Token token = scanToken();
        if(token.line != line)
        {
            printf("%4d ", token.line);
            line = token.line;
        }
        else
        {
            printf("   | ");
        }
        /**
         *  Reference : http://www.craftinginterpreters.com/scanning-on-demand.html#a-token-at-a-time
         * 
         *  That %.*s in the format string is a neat feature. 
         *  Usually, you set the output precision — the number of characters to show — by placing a number 
         *  inside the format string. Using * instead lets you pass the precision as an argument. 
         *  So that printf() call prints the first token.length characters of the string at token.start. 
         *  We need to limit the length like that because the lexeme points into the original source 
         *  string and doesn’t have a terminator at the end.
         */
        printf("%2d '%.*s'\n", token.type, token.length, token.start);

        if(token.type == TOKEN_EOF)
            break;
    }
}