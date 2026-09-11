#pragma once

#include <stdbool.h>
#include <vecfloor/vecfloor.h>
#include <vecfloor/string.h>
#include "libfloorpu/typing.h"

typedef enum
{
    TOK_EOF, // End of file
    TOK_EOL, // End of line

    TOK_IDENTIFIER,
    TOK_LIT_LABEL,
    TOK_LIT_INTEGER,
    TOK_LIT_STRING,
} TokenType;

char *token_typename(const TokenType type, bool capital);

typedef struct
{
    TokenType type;
    u32 x, y;
    union
    {
        String identifier;
        int integer;
        String string;
    } value;
} Token;

typedef struct LexerState
{
    void (*free)(struct LexerState *lexer);
    String source;
    size_t i, x, y;
    Vector labels;
    Vector tokens;
} LexerState;

bool tokenize(LexerState *lexer);
