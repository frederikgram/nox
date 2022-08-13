/**/

#include "lexer.hpp"
#include <ctype.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

// Create and append a new struct TOKEN to the linked-list of tokens.
void push(struct LEXER_STATUS *status, enum TOKEN_TYPE token, const char *value, int size)
{
    struct TOKEN *new_token = (struct TOKEN *)calloc(1,sizeof(struct TOKEN));

    new_token->type = token;
    new_token->row = status->current_row;

    // literal_value contains the value without being cast to it's proper type such as int or char.
    new_token->literal_value = (char *)calloc(1, size);
    strcpy(new_token->literal_value, value);

    if (token == T_INTEGER)
    {
        new_token->value.intval = atoi(value);
        new_token->col = status->current_col;
    }
    else if (size > 1)
    {
        new_token->value.strval = (char *)calloc(1, size);
        strncpy(new_token->value.strval, value, size);
        new_token->col = status->current_col;
    }
    else
    {
        new_token->value.charval = value[0];
        new_token->col = status->current_col;
    }

#if DEBUG
    printf("Lexing\t::\tpush\t::\t%s\n", new_token->literal_value);
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

std::map<std::string, enum TOKEN_TYPE> keywords = {
    {"if",T_IF},
    {"int",T_INT},
    {"str",T_STR},
    {"char",T_CHAR} ,
    {"func",T_FUNC} ,
    {"else",T_ELSE} ,
    {"void", T_VOID},
    {"input",T_INPUT},
    {"while",T_WHILE},
    {"print",T_PRINT},
    {"return", T_RETURN},
};

struct TOKEN *lex(char *input, int size)
{
    struct LEXER_STATUS *status = (struct LEXER_STATUS *)calloc(1,sizeof(struct LEXER_STATUS));

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
            // @TODO : Gives the wrong column and row number, might be caused in utils.cpp
        case '\n':
            status->current_row++;
            status->current_col = 0;
            break;
        case '\t':
            status->current_col += 4;
            break;
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
            buffer = (char *)calloc(1,segment_size + 2);
            buffer[segment_size + 2] = '\0';
            memcpy(buffer, input + cursor - segment_size - 1, segment_size + 2);
            push(status, T_STRING, buffer, segment_size + 2);
            segment_size = 0;
            break;

        case '\'': // Character Literal
            buffer = (char *)calloc(1,4);
            buffer[3] = '\0';
            memcpy(buffer, input + cursor, 3);
            push(status, T_CHARACTER, buffer, 3);
            cursor += 2;
            break;

        default:

            // Check for integers @TODO : Add floating point numbers
            while (isdigit(input[cursor]) == 1)
            {
                segment_size++;
                cursor++;
            }

            if (segment_size > 0)
            {
                buffer = (char *)calloc(1,segment_size + 1);
                buffer[segment_size] = '\0';
                memcpy(buffer, input + cursor - segment_size, segment_size);
                push(status, T_INTEGER, buffer, segment_size);
                segment_size = 0;
                cursor--;
                break;
            }

            // Check for Identifiers and Keywords
            while ((segment_size > 0 && input[cursor] >= 48 && input[cursor] <= 57) ||
                   (input[cursor] >= 65 && input[cursor] <= 90) || (input[cursor] >= 97 && input[cursor] <= 122) ||
                   input[cursor] == 95)
            {
                segment_size++;
                cursor++;
            }

            // Found an Identifier or Keyword
            if (segment_size > 0)
            {

                buffer = (char *)calloc(1,segment_size + 1);
                buffer[segment_size] = '\0';
                memcpy(buffer, input + cursor - segment_size, segment_size);


                // Check if it's a keyword
                if(keywords.find(buffer) != keywords.end())
                {
                    push(status, keywords[buffer], buffer, segment_size);
                }
                // It's an identifier
                else
                {
                    push(status, T_IDENTIFIER, buffer, segment_size);
                }
                
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
