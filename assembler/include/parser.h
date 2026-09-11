#pragma once

#include <stdbool.h>
#include <vecfloor/vecfloor.h>
#include <vecfloor/string.h>
#include "libfloorpu/typing.h"
#include "libfloorpu/ram.h"
#include "lexer.h"

typedef enum
{
    ARG_U8 = sizeof(u8),
    ARG_U16 = sizeof(u16),
} ArgumentType;

typedef struct
{
    const char *name;
    u8 opcode;
    u8 argc;
    ArgumentType argv[8];
} InstructionDefinition;

typedef struct ParserState
{
    void (*free)(struct ParserState *parser);
    const LexerState lexer;
    size_t ti;
    u8 buf[RAM_SIZE_ROM];
    u16 i;
} ParserState;

bool parse(ParserState *parser);
