#include <stdlib.h>
#include <stdio.h>
#include "lexer.h"
#include "parser.h"

int main(int argc, const char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input.asm> <output.rom>\n", argv[0]);
        return 1;
    }

    FILE *infile = fopen(argv[1], "rb");
    if (!infile)
    {
        fprintf(stderr, "Cannot open %s for reading!\n", argv[1]);
        return 1;
    }

    fseek(infile, 0, SEEK_END);
    long insize = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    char *source = malloc(insize + 1);
    fread(source, insize, 1, infile);
    source[insize] = '\0';
    fclose(infile);

    LexerState lexer = {.source = str_ofcstr(source)}; // "_start:\n\tpushl 0xFF\n\tpushl 0\n\tjump _start\n\tfah\n"
    if (!tokenize(&lexer))
        return 1;

    for (vec_size i = 0; i < lexer.tokens.length; i++)
    {
        Token *token = vec_get(lexer.tokens, i);
        printf("(%u:%u) ", token->y + 1, token->x + 1);
        switch (token->type)
        {
        case TOK_EOF:
        {
            printf("EOF (end of file)\n");
            break;
        }

        case TOK_EOL:
        {
            printf("EOL (end of line) -> \\n\n");
            break;
        }

        case TOK_IDENTIFIER:
        {
            char value[4096];
            str_tocstr(token->value.identifier, value);
            printf("Identifier -> %s\n", value);
            break;
        }

        case TOK_LIT_LABEL:
        {
            char value[4096];
            str_tocstr(token->value.identifier, value);
            printf("Label -> %s\n", value);
            break;
        }

        case TOK_LIT_INTEGER:
        {
            printf("Integer -> %i\n", token->value.integer);
            break;
        }

        case TOK_LIT_STRING:
        {
            char value[4096];
            str_tocstr(token->value.string, value);
            printf("String -> \"%s\"\n", value);
            break;
        }

        default:
        {
            fprintf(stderr, "Invalid token! %u\n", token->type);
            break;
        }
        }
    }

    ParserState parser = {.lexer = lexer};
    if (!parse(&parser))
        return 1;

    FILE *outfile = fopen(argv[2], "wb");
    if (!outfile)
    {
        fprintf(stderr, "Cannot open %s for writing!\n", argv[2]);
        return 1;
    }

    fwrite(parser.buf, RAM_SIZE_ROM, 1, outfile);
    fclose(outfile);

    parser.free(&parser);
    lexer.free(&lexer);
    return 0;
}
