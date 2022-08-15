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
void push(struct LEXER_STATUS *status, enum TOKEN_TYPE token, const std::string value, int size)
{
    struct TOKEN *new_token = (struct TOKEN *)calloc(1,sizeof(struct TOKEN));

    new_token->type = token;
    new_token->row = status->current_row;

    // literal_value contains the value without being cast to it's proper type such as int or char.
    new_token->literal_value = value;

    if (token == T_INTEGER)
    {
        new_token->value.intval = atoi(value.c_str());
        new_token->col = status->current_col;
    }
    else if (token == T_IDENTIFIER || token == T_STRING)
    {
        new_token->value.strval = value;
        new_token->col = status->current_col;
    }

    // @NOTE : This is somewhat of a hack, keywords would be given a charval representing keyword[0] and not a strval. This is fine as we use token->literal_value but not ideal anyhow.
    else
    { 
        new_token->value.charval = value[0];
        new_token->col = status->current_col;
    }

    printf("Lexing\t::\tpush\t::\t%s\n", new_token->literal_value.c_str());

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

std::map<std::string, enum TOKEN_TYPE> operators = {

    // Dual Character Operators
    {"==",T_EQ},
    {"!=",T_NEQ},
    {"<",T_LESS},
    {">",T_GREAT},
    {"<=",T_LEQ},
    {">=",T_GEQ},
    {"&&",T_LAND},
    {"||",T_LOR},

    // Single Character Operators
    {"+",T_ADD},
    {"-",T_SUB},
    {"*",T_STAR},
    {"%",T_MOD},
    {"=",T_ASSIGN},
    {"!",T_BANG},
    {"&",T_AMP},
    {"|",T_PIPE},
    {".",T_DOT},
    {",",T_COMMA},
    {":",T_COLON},
    {"(",T_LPARENS},
    {")",T_RPARENS},
    {"[",T_LBRACKET},
    {"]",T_RBRACKET},
    {"{",T_LBRACE},
    {"}",T_RBRACE},
    {";",T_SEMICOLON},

};


struct TOKEN *lex(char *input, int size)
{
    struct LEXER_STATUS *status = (struct LEXER_STATUS *)calloc(1,sizeof(struct LEXER_STATUS));

    int segment_size = 0;
    int cursor = 0;
    char *buffer;

    while (cursor < size)
    {

        // Check for both single-character and compound operators such as '+', '-', '==', and '&&'
        for (auto it = operators.begin(); it != operators.end(); it++)
        {
            if (cursor < size - 1 && it->first.length() == 2 && input[cursor] == it->first[0] && input[cursor + 1] == it->first[1]){
                push(status, it->second, it->first.c_str(), 2);
                cursor += 1;
                // We only increment by one as breaking the loop will increment by one.
                break;
            } else if(input[cursor] == it->first[0]) {
                push(status, it->second, it->first.c_str(), 1);
                // We don't increment the cursor as breaking the loop will do that for us.
                break;
            }
        }
  
        // Whitespaces, tabs, newlines, and comments. 
        switch (input[cursor])
        {
            // @TODO : Gives the wrong column and row number, might be caused in utils.cpp
        case '\n':
            status->current_row++;
            status->current_col = 0;
            break;
        case '\t':
            status->current_col++;
            break;
        case '\v':
        case '\r':
        case ' ':
            status->current_col++;
            break;

        // Start of comment '// foo bar'
        case '/':
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

  
        // Start of a string literal
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

        // Character Literal
        case '\'': 
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

            // Attempt to push an integer literal if one is found
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
                
                // It's an identifier
                } else
                {
                    push(status, T_IDENTIFIER, buffer, segment_size);
                }
                
                segment_size = 0;
                cursor--;
                break;
            }

            break;
        }

        // @TODO : Maybe we should free the buffer here?
        cursor++;
    }

    return status->head;
};
