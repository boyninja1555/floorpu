#include "parser.h"
#include <stdio.h>
#include "libfloorpu/cpu.h"

void parser_free(ParserState *parser)
{
}

static const size_t instructionc = 23;
static const InstructionDefinition instructionv[] = {
    {"halt", OP_HALT, 0, {}},
    {"jump", OP_JUMP, 1, {ARG_U16}},
    {"jumpiz", OP_JUMPIZ, 2, {ARG_U16, ARG_U8}},
    {"jumpnz", OP_JUMPNZ, 2, {ARG_U16, ARG_U8}},
    {"jumpr", OP_JUMPR, 2, {ARG_U8, ARG_U8}},
    {"jumpriz", OP_JUMPRIZ, 2, {ARG_U8, ARG_U8}},
    {"jumprnz", OP_JUMPRNZ, 2, {ARG_U8, ARG_U8}},
    {"set", OP_SET, 2, {ARG_U8, ARG_U8}},
    {"copy", OP_COPY, 2, {ARG_U8, ARG_U8}},
    {"load", OP_LOAD, 2, {ARG_U16, ARG_U8}},
    {"loadp", OP_LOAD, 3, {ARG_U8, ARG_U8, ARG_U8}},
    {"store", OP_STORE, 2, {ARG_U8, ARG_U16}},
    {"storep", OP_STORE, 3, {ARG_U8, ARG_U8, ARG_U8}},
    {"storel", OP_STOREL, 2, {ARG_U8, ARG_U16}},
    {"push", OP_PUSH, 1, {ARG_U8}},
    {"pushl", OP_PUSHL, 1, {ARG_U8}},
    {"pusha", OP_PUSHA, 1, {ARG_U16}},
    {"pop", OP_POP, 1, {ARG_U8}},
    {"popa", OP_POPA, 2, {ARG_U8, ARG_U8}},
    {"add", OP_ADD, 3, {ARG_U8, ARG_U8, ARG_U8}},
    {"sub", OP_SUB, 3, {ARG_U8, ARG_U8, ARG_U8}},
    {"mul", OP_MUL, 3, {ARG_U8, ARG_U8, ARG_U8}},
    {"div", OP_DIV, 3, {ARG_U8, ARG_U8, ARG_U8}},
};

bool consume_token(ParserState *parser, Token *token, const TokenType type)
{
    if (parser->ti >= parser->lexer.tokens.length)
    {
        fprintf(stderr, "(%u:%u) Ran out of tokens while expecting one of type %s!\n", token->y + 1, token->x + 1, token_typename(type, false));
        return false;
    }

    Token *i_token = vec_get(parser->lexer.tokens, parser->ti++);
    if (i_token->type != type)
    {
        fprintf(stderr, "(%u:%u) Expected token of type %s, found %s!\n", token->y + 1, token->x + 1, token_typename(type, false), token_typename(i_token->type, false));
        return false;
    }

    *token = *i_token;
    return true;
}

bool resolve_label(ParserState *parser, Vector label_addresses, String name, u16 *address)
{
    for (vec_size i = 0; i < parser->lexer.labels.length; i++)
    {
        String *label = vec_get(parser->lexer.labels, i);
        if (!str_equals(*label, name))
            continue;

        *address = *(u16 *)vec_get(label_addresses, i);
        return true;
    }

    return false;
}

bool parse(ParserState *parser)
{
    Vector label_addresses = vec_empty(sizeof(u16));
    Token *token = &(Token){.type = TOK_EOL};
    parser->free = parser_free;
    parser->ti = 0;
    parser->i = 0;

    {
        u16 address = 0;
        for (vec_size ti = 0; ti < parser->lexer.tokens.length; ti++)
        {
            token = vec_get(parser->lexer.tokens, ti);

            if (token->type == TOK_LIT_LABEL)
            {
                vec_append(&label_addresses, &address);

                char identifier[4096];
                str_tocstr(token->value.identifier, identifier);
                printf("%s -> 0x%04X\n", identifier, address);
                continue;
            }

            if (token->type != TOK_IDENTIFIER)
                continue;

            for (size_t i = 0; i < instructionc; i++)
            {
                const InstructionDefinition *definition = &instructionv[i];
                if (!str_equals_cstr(token->value.identifier, definition->name))
                    continue;

                address++;
                for (u8 j = 0; j < definition->argc; j++)
                    address += definition->argv[j] == ARG_U16 ? 2 : 1;
                ti += definition->argc;
                break;
            }
        }
    }

    token = &(Token){.type = TOK_EOL};
    parser->ti = 0;
    parser->i = 0;

    while (token->type != TOK_EOF)
    {
        token = vec_get(parser->lexer.tokens, parser->ti++);
        switch (token->type)
        {
        case TOK_EOF:
            continue;
        case TOK_EOL:
            continue;
        case TOK_LIT_LABEL:
            continue;

        case TOK_IDENTIFIER:
        {
            bool is_valid = false;
            for (size_t i = 0; i < instructionc; i++)
            {
                const InstructionDefinition *definition = &instructionv[i];
                if (!str_equals_cstr(token->value.identifier, definition->name))
                    continue;

                parser->buf[parser->i++] = definition->opcode;
                for (u8 j = 0; j < definition->argc; j++)
                {
                    Token argument;
                    switch (definition->argv[j])
                    {
                    case ARG_U8:
                    {
                        if (!consume_token(parser, &argument, TOK_LIT_INTEGER))
                            return false;

                        if (argument.value.integer < 0 || argument.value.integer > UINT8_MAX)
                        {
                            fprintf(stderr, "(%u:%u) %d does not fit inside 8 bits!\n", argument.y + 1, argument.x + 1, argument.value.integer);
                            return false;
                        }

                        parser->buf[parser->i++] = argument.value.integer;
                        break;
                    }

                    case ARG_U16:
                    {
                        if (parser->ti >= parser->lexer.tokens.length)
                        {
                            fprintf(stderr, "(%u:%u) Ran out of tokens while expecting a 16-bit operand!\n", token->y + 1, token->x + 1);
                            return false;
                        }

                        Token *argument = vec_get(parser->lexer.tokens, parser->ti++);
                        if (argument->type == TOK_LIT_INTEGER)
                        {
                            if (argument->value.integer > UINT16_MAX)
                            {
                                fprintf(stderr, "(%u:%u) %d does not fit inside 16 bits!\n", argument->y + 1, argument->x + 1, argument->value.integer);
                                return false;
                            }

                            parser->buf[parser->i++] = argument->value.integer & 0xFF;
                            parser->buf[parser->i++] = argument->value.integer >> 8;
                        }
                        else if (argument->type == TOK_IDENTIFIER)
                        {
                            u16 address;
                            if (!resolve_label(parser, label_addresses, argument->value.identifier, &address))
                            {
                                char identifier[4096];
                                str_tocstr(argument->value.identifier, identifier);
                                fprintf(stderr, "(%u:%u) Unknown label %s!\n", argument->y + 1, argument->x + 1, identifier);
                                return false;
                            }

                            parser->buf[parser->i++] = address & 0xFF;
                            parser->buf[parser->i++] = address >> 8;
                        }
                        else
                        {
                            fprintf(stderr, "(%u:%u) Expected address, found %s!\n", argument->y + 1, argument->x + 1, token_typename(argument->type, false));
                            return false;
                        }

                        break;
                    }
                    }
                }

                is_valid = true;
                break;
            }

            if (!is_valid)
            {
                char identifier[32];
                str_tocstr(token->value.identifier, identifier);
                fprintf(stderr, "(%u:%u) Unknown instruction %s!\n", token->y + 1, token->x + 1, identifier);
                return false;
            }

            continue;
        }

        default:
        {
            fprintf(stderr, "(%u:%u) Unexpected token of type %s!\n", token->y + 1, token->x + 1, token_typename(token->type, false));
            return false;
        }
        }
    }

    return true;
}
