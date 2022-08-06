/**/

#include "lexer.hpp"
#include <ctype.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

using namespace std;
// Create and append a new struct TOKEN to the linked-list of tokens.
void push(struct LEXER_STATUS *status, enum TOKEN_TYPE token, char *value, int size)
{
    struct TOKEN *new_token = (struct TOKEN *)malloc(sizeof(struct TOKEN));

    new_token->type = token;
    new_token->row = status->current_row;

    // literal_value contains the value without being cast to it's proper type such as int or char.
    new_token->literal_value = (char *)malloc(size);
    strcpy(new_token->literal_value, value);

    if (token == T_INTEGER)
    {
        new_token->value.intval = atoi(value);
        new_token->col = status->current_col;
    }
    else if (size > 1)
    {
        new_token->value.strval = (char *)malloc(size);
        strncpy(new_token->value.strval, value, size);
        new_token->col = status->current_col;
    }
    else
    {
        new_token->value.charval = value[0];
        new_token->col = status->current_col;
    }

#if DEBUG
    printf("lexer.c :: push :: %s\n", new_token->literal_value);
#endif

    status->current_col += size;

    // Maintain and update linked-list order
    if (status->head == NULL)
    {
        status->head = new_token;
        status->tail = status->head;
    }
    else
    {
        status->tail->next = new_token;
        new_token->prev = status->tail;
        status->tail = new_token;
    }
}

struct TOKEN *lex(char *input, int size)
{
    struct LEXER_STATUS *status = (struct LEXER_STATUS *)malloc(sizeof(struct LEXER_STATUS));

    int segment_size = 0;
    int cursor = 0;
    char *buffer;

    while (cursor < size)
    {
        // Check for compound operators such as '==' and '&&'
        if (cursor < size - 1)
        {

            if (strncmp("==", input + cursor, 2) == 0)
            {
                push(status, T_EQ, "==", 2);
                cursor += 2;
            }
            if (strncmp("<=", input + cursor, 2) == 0)
            {
                push(status, T_LEQ, "<=", 2);
                cursor += 2;
            }
            if (strncmp(">=", input + cursor, 2) == 0)
            {
                push(status, T_GEQ, ">=", 2);
                cursor += 2;
            }
            if (strncmp("!=", input + cursor, 2) == 0)
            {
                push(status, T_NEQ, "!=", 2);
                cursor += 2;
            }
            if (strncmp("&&", input + cursor, 2) == 0)
            {
                push(status, T_AND, "&&", 2);
                cursor += 2;
            }
            if (strncmp("||", input + cursor, 2) == 0)
            {
                push(status, T_OR, "||", 2);
                cursor += 2;
            }
        }

        switch (input[cursor])
        {
        case '\n':
            status->current_row++;
            status->current_col = 0;
            break;
        case '\t':
        case '\v':
        case '\r':
        case ' ':
            status->current_col++;
            break;

        case '/':

            // Start of comment '// foo bar'
            if (cursor + 1 <= size && input[cursor + 1] == '/')
            {
                while (input[cursor] != '\n')
                {
                    cursor++;
                    status->current_row++;
                }
                status->current_col = 0;
            }
            else
            {
                push(status, T_DIV, "/", 1);
            }
            break;

        case '-':
            push(status, T_SUB, "-", 1);
            break;
        case ',':
            push(status, T_COMMA, ",", 1);
            break;
        case ';':
            push(status, T_SEMICOLON, ";", 1);
            break;
        case ':':
            push(status, T_COLON, ":", 1);
            break;
        case '!':
            push(status, T_BANG, "!", 1);
            break;
        case '.':
            push(status, T_DOT, ".", 1);
            break;
        case '(':
            push(status, T_LPARENS, "(", 1);
            break;
        case ')':
            push(status, T_RPARENS, ")", 1);
            break;
        case '[':
            push(status, T_LBRACKET, "[", 1);
            break;
        case ']':
            push(status, T_RBRACKET, "]", 1);
            break;
        case '{':
            push(status, T_LBRACE, "{", 1);
            break;
        case '}':
            push(status, T_RBRACE, "}", 1);
            break;
        case '*':
            push(status, T_STAR, "*", 1);
            break;
        case '&':
            push(status, T_AMP, "&", 1);
            break;
        case '%':
            push(status, T_MOD, "%%", 1);
            break;
        case '+':
            push(status, T_ADD, "+", 1);
            break;
        case '<':
            push(status, T_LESS, "<", 1);
            break;
        case '=':
            push(status, T_ASSIGN, "=", 1);
            break;
        case '>':
            push(status, T_GREAT, ">", 1);
            break;
        case '|':
            push(status, T_PIPE, "|", 1);
            break;

        // Start of string @TODO : Surely there is a prettier way to do this...
        case '"':
            while (cursor + 1 <= size && (input[++cursor] != '"' || input[cursor - 1] == '\\'))
            {
                segment_size++;
            }
            buffer = (char *)malloc(segment_size + 2);
            buffer[segment_size + 2] = '\0';
            memcpy(buffer, input + cursor - segment_size - 1, segment_size + 2);
            push(status, T_STRING, buffer, segment_size + 2);
            segment_size = 0;
            break;

        case '\'': // Character Literal
            buffer = (char *)malloc(4);
            buffer[3] = '\0';
            memcpy(buffer, input + cursor, 3);
            push(status, T_CHARACTER, buffer, 3);
            cursor += 2;
            break;

        default:

            // Check for keywords @NOTE : we subtract one from the lenght of the keyword since when we break we
            // increment our cursor by one automatically, meaning that we'd go out-of-bounds if we didn't subtract.
            if (strncmp("if", input + cursor, 2) == 0)
            {
                cursor += 1;
                push(status, T_IF, "if", 2);
                break;
            }
            if (strncmp("int", input + cursor, 3) == 0)
            {
                cursor += 2;
                push(status, T_INT, "int", 3);
                break;
            }
            if (strncmp("str", input + cursor, 3) == 0)
            {
                cursor += 2;
                push(status, T_STR, "str", 3);
                break;
            }
            if (strncmp("char", input + cursor, 4) == 0)
            {
                cursor += 3;
                push(status, T_CHAR, "char", 4);
                break;
            }
            if (strncmp("func", input + cursor, 4) == 0)
            {
                cursor += 3;
                push(status, T_FUNC, "func", 4);
                break;
            }
            if (strncmp("else", input + cursor, 4) == 0)
            {
                cursor += 3;
                push(status, T_ELSE, "else", 4);
                break;
            }
            if (strncmp("input", input + cursor, 5) == 0)
            {
                cursor += 4;
                push(status, T_INPUT, "input", 5);
                break;
            }
            if (strncmp("while", input + cursor, 5) == 0)
            {
                cursor += 4;
                push(status, T_WHILE, "while", 5);
                break;
            }
            if (strncmp("print", input + cursor, 6) == 0)
            {
                cursor += 5;
                push(status, T_PRINT, "print", 6);
                break;
            }

            if (strncmp("return", input + cursor, 6) == 0)
            {
                cursor += 5;
                push(status, T_RETURN, "return", 6);
                break;
            }
            if (strncmp("include", input + cursor, 7) == 0)
            {
                cursor += 6;
                push(status, T_RETURN, "include", 7);
                break;
            }

            // Check for integers @TODO : Add floating point numbers
            while (isdigit(input[cursor]) == 1)
            {
                segment_size++;
                cursor++;
            }

            if (segment_size > 0)
            {
                buffer = (char *)malloc(segment_size + 1);
                buffer[segment_size] = '\0';
                memcpy(buffer, input + cursor - segment_size, segment_size);
                push(status, T_INTEGER, buffer, segment_size);
                segment_size = 0;
                cursor--;
                break;
            }

            // Check for Identifier
            while ((segment_size > 0 && input[cursor] >= 48 && input[cursor] <= 57) ||
                   (input[cursor] >= 65 && input[cursor] <= 90) || (input[cursor] >= 97 && input[cursor] <= 122) ||
                   input[cursor] == 95)
            {
                segment_size++;
                cursor++;
            }

            // Found an Identifier
            if (segment_size > 0)
            {
                buffer = (char *)malloc(segment_size + 1);
                buffer[segment_size] = '\0';
                memcpy(buffer, input + cursor - segment_size, segment_size);
                push(status, T_IDENTIFIER, buffer, segment_size);
                segment_size = 0;
                cursor--;
                break;
            }

            break;
        }
        cursor++;
    }

    return status->head;
};
