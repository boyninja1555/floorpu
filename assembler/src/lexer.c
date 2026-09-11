#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>

char *token_typename(const TokenType type, bool capital)
{
    switch (type)
    {
    case TOK_EOF:
        return "EOF";
    case TOK_EOL:
        return "EOL";
    case TOK_IDENTIFIER:
        return capital ? "Identifier" : "identifier";
    case TOK_LIT_LABEL:
        return capital ? "Literal label" : "literal label";
    case TOK_LIT_INTEGER:
        return capital ? "Literal integer" : "literal integer";
    case TOK_LIT_STRING:
        return capital ? "Literal string" : "literal string";
    default:
        return "NULL";
    }
}

void lexer_free(LexerState *lexer)
{
    str_free(&lexer->source);
    vec_free(&lexer->tokens);
}

void mk_token(LexerState *lexer, Token *token)
{
    token->x = lexer->x - ((token->type == TOK_IDENTIFIER || token->type == TOK_LIT_LABEL) ? token->value.identifier.length + (token->type == TOK_LIT_LABEL ? 1 : 0) : 0);
    token->y = lexer->y;
    vec_append(&lexer->tokens, token);
}

void chomp_leftovers(LexerState *lexer, String *temp)
{
    if (temp->length > 0)
    {
        if (str_at(*temp, temp->length - 1) == ':')
        {
            String label = str_dup(*temp);
            label.length--;
            vec_append(&lexer->labels, &label);
            mk_token(lexer, &(Token){.type = TOK_LIT_LABEL, .value = {.identifier = label}});
        }
        else
            mk_token(lexer, &(Token){.type = TOK_IDENTIFIER, .value = {.identifier = str_dup(*temp)}});
        temp->length = 0;
    }
}

bool tokenize(LexerState *lexer)
{
    lexer->free = lexer_free;
    lexer->x = 0;
    lexer->y = 0;
    lexer->labels = vec_empty(sizeof(String));
    lexer->tokens = vec_empty(sizeof(Token));
    String temp = str_empty();

    for (lexer->i = 0; lexer->i < lexer->source.length; lexer->i++)
    {
        char c = str_at(lexer->source, lexer->i);

        if (c == ' ' || c == '\t' || c == '\r')
        {
            chomp_leftovers(lexer, &temp);
            lexer->x++;
            continue;
        }

        if (c == '\n')
        {
            chomp_leftovers(lexer, &temp);
            mk_token(lexer, &(Token){.type = TOK_EOL});
            lexer->x = 0;
            lexer->y++;
            continue;
        }

        if (c == ';')
        {
            while (lexer->i < lexer->source.length && str_at(lexer->source, lexer->i) != '\n')
                lexer->i++;
            lexer->i--;
            continue;
        }

        if (c == '0')
        {
            if (temp.length > 0)
            {
                str_append(&temp, c);
                continue;
            }

            if (lexer->i + 1 < lexer->source.length)
            {
                char prefix = str_at(lexer->source, lexer->i + 1);
                if (prefix == 'x' || prefix == 'X')
                {
                    lexer->i += 2;
                    lexer->x += 2;

                    while (lexer->i < lexer->source.length)
                    {
                        c = str_at(lexer->source, lexer->i);
                        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
                            break;

                        str_append(&temp, c);
                        lexer->i++;
                        lexer->x++;
                    }
                    lexer->i--;
                    lexer->x--;

                    int integer = 0;
                    for (vec_size i = 0; i < temp.length; i++)
                    {
                        c = str_at(temp, i);
                        integer *= 16;
                        if (c >= '0' && c <= '9')
                            integer += c - '0';
                        else if (c >= 'a' && c <= 'f')
                            integer += c - 'a' + 10;
                        else
                            integer += c - 'A' + 10;
                    }

                    mk_token(lexer, &(Token){.type = TOK_LIT_INTEGER, .value = {.integer = integer}});
                    temp.length = 0;
                    continue;
                }
            }
        }

        if (c == 'r' && lexer->i + 1 < lexer->source.length && str_at(lexer->source, lexer->i + 1) >= '0' && str_at(lexer->source, lexer->i + 1) <= '9')
        {
            if (temp.length > 0)
            {
                str_append(&temp, c);
                lexer->x++;
                continue;
            }

            lexer->i++;
            lexer->x++;
            int integer = 0;
            while (lexer->i < lexer->source.length)
            {
                c = str_at(lexer->source, lexer->i);
                if (c < '0' || c > '9')
                    break;

                integer = integer * 10 + c - '0';
                lexer->i++;
                lexer->x++;
            }

            lexer->i--;
            lexer->x--;
            mk_token(lexer, &(Token){.type = TOK_LIT_INTEGER, .value = {.integer = integer}});
            continue;
        }

        if (c >= '0' && c <= '9')
        {
            if (temp.length > 0)
            {
                str_append(&temp, c);
                continue;
            }

            while (lexer->i < lexer->source.length)
            {
                c = str_at(lexer->source, lexer->i);
                if (c < '0' || c > '9')
                    break;

                str_append(&temp, c);
                lexer->i++;
                lexer->x++;
            }
            lexer->i--;
            lexer->x--;

            int integer = 0;
            for (vec_size i = 0; i < temp.length; i++)
                integer = integer * 10 + str_at(temp, i) - '0';
            mk_token(lexer, &(Token){.type = TOK_LIT_INTEGER, .value = {.integer = integer}});
            temp.length = 0;
            continue;
        }

        if (c == '"')
        {
            chomp_leftovers(lexer, &temp);
            lexer->i++;
            lexer->x++;
            while (lexer->i < lexer->source.length)
            {
                c = str_at(lexer->source, lexer->i);
                if (c == '"')
                    break;

                str_append(&temp, c);
                lexer->i++;
                lexer->x++;
            }

            if (lexer->i >= lexer->source.length)
            {
                fprintf(stderr, "(%zu:%zu) Reached end of file before reaching the end of a string!\n", lexer->y + 1, lexer->x + 1);
                str_free(&temp);
                return false;
            }

            mk_token(lexer, &(Token){.type = TOK_LIT_STRING, .value = {.string = str_dup(temp)}});
            temp.length = 0;
            continue;
        }

        str_append(&temp, c);
        lexer->x++;
    }

    chomp_leftovers(lexer, &temp);
    mk_token(lexer, &(Token){.type = TOK_EOF});
    str_free(&temp);
    return true;
}
