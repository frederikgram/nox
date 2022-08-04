/**/

#include "parser.hpp"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.hpp"


// Allocate and populate a new AST_NODE
struct AST_NODE * make_node(struct PARSER_STATUS * status, enum AST_NODE_TYPE type, struct AST_NODE * parent) {
    struct AST_NODE * node = (struct AST_NODE *) malloc(sizeof(struct AST_NODE));
    node->type = type;
    node->parent = parent;

    if (type == A_BLOCK) {
        node->max_num_statments = 0;
        node->num_statements = 0;
    }

    node->token = status->current;

    return node;
}

// Progress Linked-List via. status->current
void consume(struct PARSER_STATUS * status) {
    if (status->current != NULL && status->current->next != NULL) {
        status->current = status->current->next;
        status->vpos++;
    }
}

// Progress Linked-List via. status->current if the given TOKEN_TYPE type is equivalent to status->current->type
int consume_if(struct PARSER_STATUS * status, enum TOKEN_TYPE type) {
    if (status->current == NULL) {
        return 0;
    }

    if (status->current->type == type) {
        consume(status);
        return 1;
    }
    return 0;
}


// ²TODO : this is temporary, it sucks ! Print both the row and its two surrounding rows in both directions, with a row
// containing the given error message pointing at the proper column
void print_error(struct PARSER_STATUS * status, char * message, ...) {
    int row_number = 0;
    int shown = 0;
    int i = 0;
    va_list lst;
    printf("---- PARSING ERROR ----\nline %d| ", 0);
    while (i < status->input_size && row_number <= status->current->row + 3) {
        // Print the error message below the proper row
        if (row_number == status->current->row + 1 && shown == 0) {
            for (int j = 0; j < status->current->col + 6 + (int)log10(row_number); j++) {
                printf(" ");
            }
            printf("^ ");
            va_start(lst, message);
            printf(message, lst);
            va_end(lst);
            printf(" at line '%d', column '%d'\n", status->current->row, status->current->col);
            shown = 1;
        }

        // Print surrounding rows
        if (row_number >= status->current->row - 3 && row_number <= status->current->row + 3 &&
            (row_number == status->current->row + 1 && shown == 1) == 0) {
            printf("%c", status->input[i]);
        }

        // Keep track of row number
        if (status->input[i] == '\n' && i - 1 >= 0 && status->input[i - 1] != '\\') {
            row_number++;
            if (row_number == status->current->row + 1 && shown == 0) {
                continue;
            }
            if (row_number == status->current->row + 4) {
                break;
            }
            printf("line %d| ", row_number - shown);
        }

        i++;
    }

    printf("-----------------------\n");
}

void print_error_exit(struct PARSER_STATUS * status, char * message, ...) {
    va_list lst;
    va_start(lst, message);
    print_error(status, message, lst);
    va_end(lst);
    exit(-1);
}

// Exit with flag -1 if status->current->type != the given TOKEN_TYPE type
void consume_assert(struct PARSER_STATUS * status, enum TOKEN_TYPE type, char * format, ...) {
    if (consume_if(status, type) == 1) {
        return;
    }

    va_list lst;
    va_start(lst, format);
    print_error(status, format, lst);
    va_end(lst);
    exit(-1);
}


enum AST_NODE_TYPE token_type_to_node_type(enum TOKEN_TYPE type) {
    switch (type) {
        case T_ASSIGN:
            return A_ASSIGN;
        case T_AEQ:
            return A_AEQ;
        case T_SEQ:
            return A_SEQ;
        case T_LESS:
            return A_LESS;
        case T_GREAT:
            return A_GREAT;
        case T_GEQ:
            return A_GEQ;
        case T_LEQ:
            return A_LEQ;
        case T_EQ:
            return A_EQ;
    }
}

//@TODO : Convert to C++ std::vector
void add_arg_to_node(struct AST_NODE * arg, struct AST_NODE * node) {
    if (node->num_args + 1 > node->max_num_statments) {
        node->args = (struct AST_NODE **) realloc(node->args, sizeof(struct AST_NODE *) * (node->max_num_statments + 8));
        node->max_num_statments += 8;
    }

    node->args[node->num_args] = arg;
    node->num_args++;
}

//@TODO : Convert to C++ std::vector
void add_stmt_to_block(struct AST_NODE * stmt, struct AST_NODE * block) {
    if (block->num_statements + 1 > block->max_num_statments) {
        block->statements = (struct AST_NODE **) realloc(block->statements, sizeof(struct AST_NODE *) * (block->max_num_statments + 8));
        block->max_num_statments += 8;
    }

    block->statements[block->num_statements] = stmt;
    block->num_statements++;
}

struct AST_NODE * parse_block(struct PARSER_STATUS * status, struct AST_NODE * parent) {
    struct AST_NODE * node = make_node(status, A_BLOCK, parent);
    struct AST_NODE * stmt;
    consume(status);  // Consume '{', we know it exists as 'parse_block()' was called in the first place
    while ((stmt = parse_statement(status, node)) != NULL) {
        add_stmt_to_block(stmt, node);
    };

    consume_assert(status, T_RBRACE, "Expected '}' but received '%s'", status->current->literal_value);
    return node;
}

struct AST_NODE * parse_type(struct PARSER_STATUS * status, struct AST_NODE * parent) {
    struct AST_NODE * node;
    switch (status->current->type) {
        case T_INT:
        case T_STR:
            break;
        default:
            return NULL;
    }

    node = make_node(status, token_type_to_node_type(status->current->type), parent);
    consume(status);
    return node;
}

struct AST_NODE * parse_assignment(struct PARSER_STATUS * status, struct AST_NODE * lhs, struct AST_NODE * parent) {
    struct AST_NODE * node = make_node(status, A_ASSIGN, parent);

    printf("Parsing Assignment Statement\n");

    // (identifier): type = value
    lhs->parent = node;
    node->lhs = lhs;

    // identifier: (type) = value
    consume_assert(status, T_COLON, "Expected a ':' before type declaration of variable");
    node->vartype = parse_type(status, node);
    if (node->vartype == NULL) {
        print_error_exit(status, "No type specified during variable declaration");
    }

    // identifier: type (= value) : OPTIONAL
    if (consume_if(status, T_ASSIGN) == 1) {
        node->rhs = parse_expression(status, node);
    } else {
        node->rhs = NULL;
    }

    return node;
}

struct AST_NODE * parse_statement(struct PARSER_STATUS * status, struct AST_NODE * parent) {
    struct AST_NODE * node = NULL;

    switch (status->current->type) {
        case T_WHILE:
        case T_IF:
            node = make_node(status, status->current->type == T_IF ? A_IF : A_WHILE, parent);
            printf("Parsing if/while Statement\n");
            consume(status);
            consume_assert(status, T_LPARENS, "Expected '(' at start of conditional");

            // Conditional
            node->conditional = parse_expression(status, node);
            consume_assert(status, T_RPARENS, "Expected ')' at end of conditional");

            // Body
            node->body = parse_block(status, node);  // parse_block() consumes both '{' and '}'

            // Handle 'else' branches for 'if' statements
            if (node->type == A_IF && status->current->type == T_ELSE) {
                node->els = parse_block(status, node);
            }

            return node;  // Return to remove the ';' requirement for 'block' statements
        case T_LBRACE:    // Dangling block
            printf("Entering block parse\n");
            node = parse_block(status, parent);
            printf("Exiting block parse\n");
            return node;  // Return to remove the ';' requirement for 'block' statements

        case T_OUTPUT:
            node = make_node(status, A_OUTPUT, parent);
            consume(status);
            printf("Parsing Output Statement\n");
            node->lhs = parse_expression(status, node);
            break;

        case T_FUNC:
            node = make_node(status, A_FUNC_DEF, parent);
            printf("Parsing Function Definition Statement\n");
            consume(status);

            // Parse Type
            node->vartype = parse_type(status, node);
            if (node->vartype == NULL) {
                print_error_exit(status, "No type specified in function definition");
            }

            // Parse Function Name
            node->name = parse_expression(status, node);
            if (node->name->type != A_IDENTIFIER) {
                print_error_exit(status, "Expected an identifier in funcion definition");
            }

            // Parse Arguments
            consume_assert(status, T_LPARENS, "Expected '(' at the beginning of argument declarations in function definition");
            struct AST_NODE *tmp_type, *tmp_expr;
            while ((tmp_type = parse_type(status, node)) != NULL) {
                tmp_expr = parse_expression(status, node);
                tmp_expr->vartype = tmp_type;
                add_arg_to_node(tmp_expr, node);

                if (consume_if(status, T_COMMA) == 0) {
                    break;
                }
            }

            consume_assert(status, T_RPARENS, "Expected ')' at the end of argument declarations in function definition");

            // Function Block
            node->block = parse_block(status, node);
            return node;  // Return to remove the ';' requirement for 'block' statements

        case T_RETURN:
            node = make_node(status, A_RETURN, parent);
            printf("Parsing Return Statement\n");
            consume(status);
            node->lhs = parse_expression(status, node);
            break;

        case T_USE:
            break;  // @TODO : Gotta write a function that reads the input, parses all its functions, variables and Use's

        default:  // Defaults to expression, returns NULL if no valid expression could be constructed
            node = parse_expression(status, parent);
            if (node == NULL) {
                break;
            }

            // Check for Assignment Statement
            if (node->type == A_IDENTIFIER && status->current->type == T_COLON) {
                node = parse_assignment(status, node, parent);
            }
            break;
    }

    if (node != NULL) {
        consume_assert(status, T_SEMICOLON, "Expected a ';' at the end of the statement");
    }
    return node;
}


//@TODO : Not finished, add floats etc.
struct AST_NODE * parse_factor(struct PARSER_STATUS * status, struct AST_NODE * parent) {
    struct AST_NODE * node;

    switch (status->current->type) {
        case T_LPARENS:
            consume(status);
            node = parse_expression(status, parent);
            consume_assert(status, T_RPARENS, "Expected a ')' at the end of a '(expr)' ");
            printf("parse_factor :: T_LPARENS :: %c\n", status->current->value.charval);
            return node;  // we return instead of breaking as we need to perform 'consume_assert()' and breaking would only
                          // perform 'consume()'
        case T_STRING:
            node = make_node(status, A_STRING, parent);
            printf("parse_factor :: T_STRING :: %s\n", status->current->value.strval);
            break;
        case T_INTEGER:
            node = make_node(status, A_INTEGER, parent);
            printf("parse_factor :: T_INTEGER :: %d\n", status->current->value.intval);
            break;
        case T_IDENTIFIER:
            node = make_node(status, A_IDENTIFIER, parent);
            printf("parse_factor :: T_IDENTIFIER :: %s\n", status->current->literal_value);
            break;
        default:
            return NULL;
    }

    consume(status);
    return node;
}

struct AST_NODE * parse_term(struct PARSER_STATUS * status, struct AST_NODE * parent) {
    struct AST_NODE * lhs = parse_factor(status, NULL);
    if (lhs == NULL) {
        return NULL;
    }

    struct AST_NODE * p;

    while (status->current->type == T_STAR || status->current->type == T_DIV) {
        p = make_node(status, status->current->type == T_STAR ? A_MUL : A_DIV, NULL);

        printf("parse_term :: %s\n", status->current->literal_value);

        consume(status);
        lhs->parent = p;
        p->lhs = lhs;
        p->rhs = parse_term(status, p);
        lhs = p;
    }

    lhs->parent = parent;
    return lhs;
}

struct AST_NODE * parse_addititive(struct PARSER_STATUS * status, struct AST_NODE * parent) {
    struct AST_NODE * lhs = parse_term(status, NULL);
    if (lhs == NULL) {
        return NULL;
    }

    struct AST_NODE * p;

    while (status->current->type == T_ADD || status->current->type == T_SUB) {
        p = make_node(status, status->current->type == T_ADD ? A_ADD : A_SUB, NULL);
        printf("parse_addititive :: %s\n", status->current->literal_value);
        consume(status);
        lhs->parent = p;
        p->lhs = lhs;
        p->rhs = parse_addititive(status, p);  // @TODO : Should this be parse_addititive  or parse_term ?
        lhs = p;
    }

    lhs->parent = parent;
    return lhs;
}


struct AST_NODE * parse_relational(struct PARSER_STATUS * status, struct AST_NODE * parent) {
    struct AST_NODE * lhs = parse_addititive(status, NULL);
    if (lhs == NULL) {
        return NULL;
    }

    struct AST_NODE * p;
    while (status->current->type == T_LESS || status->current->type == T_GREAT || status->current->type == T_LEQ ||
           status->current->type == T_GEQ || status->current->type == T_EQ) {
        p = make_node(status, token_type_to_node_type(status->current->type), NULL);
        printf("parse_relational :: %s\n", status->current->literal_value);
        consume(status);
        lhs->parent = p;
        p->lhs = lhs;
        p->rhs = parse_relational(status, p);
        lhs = p;
    }

    lhs->parent = parent;
    return lhs;
}

struct AST_NODE * parse_logical_and(struct PARSER_STATUS * status, struct AST_NODE * parent) {
    struct AST_NODE * lhs = parse_relational(status, NULL);
    if (lhs == NULL) {
        return NULL;
    }

    struct AST_NODE * p;

    while (status->current->type == T_AMP) {
        p = make_node(status, A_LAND, NULL);
        printf("parse_logical_and :: %s\n", status->current->literal_value);
        consume(status);
        lhs->parent = p;
        p->lhs = lhs;
        p->rhs = parse_logical_and(status, p);
        lhs = p;
    }

    lhs->parent = parent;
    return lhs;
}

struct AST_NODE * parse_logical_or(struct PARSER_STATUS * status, struct AST_NODE * parent) {
    struct AST_NODE * lhs = parse_logical_and(status, NULL);
    if (lhs == NULL) {
        return NULL;
    }

    struct AST_NODE * p;

    while (status->current->type == T_PIPE) {
        p = make_node(status, A_LOR, NULL);
        printf("parse_logical_or :: %s\n", status->current->literal_value);
        consume(status);
        lhs->parent = p;
        p->lhs = lhs;
        p->rhs = parse_logical_or(status, p);
        lhs = p;
    }

    lhs->parent = parent;
    return lhs;
}

struct AST_NODE * parse_expression(struct PARSER_STATUS * status, struct AST_NODE * parent) {
    struct AST_NODE * lhs = parse_logical_or(status, NULL);
    if (lhs == NULL) {
        return NULL;
    }

    struct AST_NODE * p;
    while (status->current->type == T_ASSIGN || status->current->type == T_AEQ || status->current->type == T_SEQ) {
        p = make_node(status, token_type_to_node_type(status->current->type), NULL);
        printf("parse_expression :: %s\n", status->current->literal_value);
        consume(status);
        lhs->parent = p;
        p->lhs = lhs;
        p->rhs = parse_expression(status, p);
        lhs = p;
    }

    lhs->parent = parent;
    return lhs;
}




/* Convert the linked list of tokens into an abstract syntax tree, entrypoint for the recursion */
struct AST_NODE * parse(char * input, int size, struct TOKEN * head) {
    // Initialization
    struct PARSER_STATUS * status = (struct PARSER_STATUS * )malloc(sizeof(struct PARSER_STATUS));
    struct AST_NODE * root = (struct AST_NODE *)malloc(sizeof(struct AST_NODE));
    status->head = head;
    status->current = head;
    status->vpos = 0;
    status->input = input;
    status->input_size = size;

    printf("/* BEGIN PARSING */\n");

    status->root = parse_statement(status, NULL);
    printf("/* FINISHED PARSING */\n");
}

