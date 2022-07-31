/**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "utils.c"

void pushv(struct LEXER_STATUS * status, enum TOKEN_TYPE token, char * value, int size) {

    struct TOKEN * new_token = malloc(sizeof(struct TOKEN));

    new_token->type = token;
    new_token->row = status->current_row;
    
    new_token->literal_value = malloc(size);
    strcpy(new_token->literal_value, value);

    // If a value is supplied, copy it into the new token and increment the column count.
    if (value != NULL) {

        if (token == T_INT) { 
            new_token->value.intval = atoi(value);
            new_token->col = status->current_col;
        } else if (size > 1) {
            new_token->value.strval = (char *) malloc(size);
            strncpy(new_token->value.strval, value, size);
            new_token->col = status->current_col;
        } else {
            new_token->value.charval = value[0];
            new_token->col = status->current_col;
        }

        #if DEBUG
            printf("lexer.c :: pushv :: %s\n", new_token->literal_value);
        #endif


        status->current_col += size;

    // Increment column count by a single character
    } else {
        new_token->col = ++status->current_col;
    }

    if(status->head == NULL) {
        status->head = new_token;
        status->tail = status->head;
    } else {
        status->tail->next = new_token;
        new_token->prev = status->tail;
        status->tail = new_token;
    }
}

void push(struct LEXER_STATUS * status, enum TOKEN_TYPE token)  {
    pushv(status, token, NULL, -1);
}

struct TOKEN * lex(char * input, int size) {
    
    struct LEXER_STATUS * status = malloc(sizeof(struct LEXER_STATUS));

    int cursor = 0;

    // Is this hacky?
    int segment_size = 0;
    char * buffer;

    while (cursor < size) {
        switch (input[cursor]) {
            case '\n':
                status->current_row++;
                status->current_col = 0;
                break;
            case '\t':
            case '\v':
            case '\r':
            case ' ' :
                status->current_col++;
                break;

            case '+': pushv(status, T_ADD, "+", 1);break;
            case '-': pushv(status, T_SUB, "-", 1);break;
            case '*': pushv(status, T_STAR, "*", 1);break;
            case '/':

                // Start of comment '// foo bar'
                if (cursor + 1 <= size && input[cursor + 1] == '/') {
                    while(input[cursor] != '\n') { cursor++; status->current_row++; }
                    status->current_col = 0;    
                } else {
                    push(status, T_DIV);
                }
                break;

            case '(': pushv(status, T_LPARENS,  "(", 1);break;
            case ')': pushv(status, T_RPARENS,  ")", 1);break;
            case '[': pushv(status, T_LBRACKET, "[", 1);break;
            case ']': pushv(status, T_RBRACKET, "]", 1);break;
            case '{': pushv(status, T_LBRACE,   "{", 1);break;
            case '}': pushv(status, T_RBRACE,   "}", 1);break;

            case '.': pushv(status, T_DOT,       ".", 1);break;
            case ',': pushv(status, T_COMMA,     ",", 1);break;
            case ':': pushv(status, T_COLON,     ":", 1);break;
            case ';': pushv(status, T_SEMICOLON, ";", 1);break;
 
            case '!': pushv(status, T_BANG, "!", 1); break;
            case '&': pushv(status, T_AMP,  "&", 1); break;
            case '|': pushv(status, T_PIPE, "|", 1); break;
            case '<': pushv(status, T_LESS, "<", 1); break;
            case '>': pushv(status, T_GREAT,">", 1); break;
            case '=': pushv(status, T_EQUAL,"=", 1); break;
            case '%': pushv(status, T_MOD,  "%%", 1);break;

            // Start of string @TODO : Surely there is a prettier way to do this...
            case '"': 
                while(cursor + 1 <= size && (input[++cursor] != '"' || input[cursor-1] == '\\')) { segment_size++; }
                
                buffer = (char *) malloc(segment_size + 2); 
                buffer[segment_size+2] = '\0';
                memcpy(buffer, input + cursor - segment_size - 1, segment_size+2);
                pushv(status, T_STRING, buffer, segment_size+2);
                segment_size = 0;
                break;

            default:

                // Check for integers @TODO : Add floating point numbers 
                while(is_digit(input[cursor]) == 1) { segment_size++; cursor++; }
                
                if(segment_size > 0) {
                    buffer = (char *) malloc(segment_size + 1); buffer[segment_size + 1] = '\0'; 
                    memcpy(buffer, input + cursor - segment_size, segment_size );
                    pushv(status, T_INT, buffer, segment_size);
                    segment_size = 0; cursor--;
                    break;
                }

                // Check for keywords @NOTE : we subtract one from the lenght of the keyword since whne we break we increment our cursor by one automatically, meaning that we'd go out-of-bounds if we didn't subtract.
                if(strncmp("if", input+cursor,2) == 0) { cursor+=1;pushv(status, T_IF, "if", 2);break;}
                if(strncmp("for", input+cursor, 3) == 0) { cursor+=2;pushv(status, T_FOR, "for", 3);break;}
                if(strncmp("while", input+cursor,5) == 0) { cursor+=4;pushv(status, T_WHILE, "while", 5);break;}

                break;
        }

        cursor++;
    }

    return status->head;
};
