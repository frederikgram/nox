/**/

#include "parser.hpp"
#include "lexer.hpp"
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
using namespace std;

// Allocate and populate a new AST_NODE
struct AST_NODE *make_node(struct PARSER_STATUS *status, enum AST_NODE_TYPE type, struct AST_NODE *parent)
{
    struct AST_NODE *node = (struct AST_NODE *)malloc(sizeof(struct AST_NODE));
    node->type = type;
    node->parent = parent;
    node->token = status->current;
    return node;
}
// Progress Linked-List via. status->current
void consume(struct PARSER_STATUS *status)
{
    if (status->current != NULL && status->current->next != NULL)
    {
        status->current = status->current->next;
        status->vpos++;
    }
}
// Progress Linked-List via. status->current if the given TOKEN_TYPE type is
// equivalent to status->current->type
int consume_if(struct PARSER_STATUS *status, enum TOKEN_TYPE type)
{
    if (status->current == NULL)
    {
        return 0;
    }

    if (status->current->type == type)
    {
        consume(status);
        return 1;
    }
    return 0;
}
// @TODO : this is temporary, it sucks ! Print both the row and its two
// surrounding rows in both directions, with a row containing the given error
// message pointing at the proper column
void print_error(int col, int row, const char *message, ...)
{
    int row_number = 1;
    int shown = 0;
    int i = 0;
    va_list lst;
    fprintf(stderr, "PARSING ERROR:\t");
    va_start(lst, message);
    fprintf(stderr, message, lst);
    va_end(lst);
    fprintf(stderr, " at line '%d', column '%d'\n", row - 1, col - 1);
}
// Calls print_error and exits with flag -1
void print_error_exit(int col, int row, const char *message, ...)
{
    va_list lst;
    va_start(lst, message);
    print_error(col, row, message, lst);
    va_end(lst);
    exit(-1);
}

// Exit with flag -1 if status->current->type != the given TOKEN_TYPE type
void consume_assert(struct PARSER_STATUS *status, enum TOKEN_TYPE type, const char *format, ...)
{
    if (consume_if(status, type) == 1)
    {
        return;
    }

    va_list lst;
    va_start(lst, format);
    print_error(status->current->col, status->current->row, format, lst);
    va_end(lst);
    exit(-1);
}
// Converts _some_ enum TOKEN_TYPE's to enum AST_NODE_TYPE's
enum AST_NODE_TYPE token_type_to_node_type(enum TOKEN_TYPE type)
{
    switch (type)
    {
    case T_ASSIGN:
        return A_ASSIGN;
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
    case T_NEQ:
        return A_NEQ;
    case T_INT:
        return A_INT;
    case T_STR:
        return A_STR;
    case T_CHAR:
        return A_CHAR;
    default:
        break;
    }

    return A_EMPTY;
}
// Fetches a list of struct AST_NODE * statements and collects them inside an A_BLOCK node, pushing each statement to
// node->statements
struct AST_NODE *parse_block(struct PARSER_STATUS *status, struct AST_NODE *parent)
{
    struct AST_NODE *node = make_node(status, A_BLOCK, parent);
    struct AST_NODE *stmt;
    consume(status); // Consume '{', we know it exists as 'parse_block()' was
                     // called in the first place
    while ((stmt = parse_statement(status, node)) != NULL)
    {
        node->statements.push_back(stmt);
    };

    consume_assert(status, T_RBRACE, "Expected '}' but received '%s'", status->current->literal_value);
    return node;
}
// @TODO : Add Floats
struct AST_NODE *parse_type(struct PARSER_STATUS *status, struct AST_NODE *parent)
{

    printf("Parsing Type\n");

    struct AST_NODE *node;
    switch (status->current->type)
    {
    case T_INT:
    case T_STR:
    case T_CHAR:
        break;
    default:
        return NULL;
    }

    node = make_node(status, token_type_to_node_type(status->current->type), parent);
    consume(status);
    return node;
}
// Creates an AST_NODE * of type A_IDENTIFIER if the status->current is a T_IDENTIFIER, if one was found, we consume it.
struct AST_NODE *parse_identifier(struct PARSER_STATUS *status, struct AST_NODE *parent)
{

    if (status->current == NULL || status->current->type != T_IDENTIFIER)
    {
        return NULL;
    }

    struct AST_NODE *ident = make_node(status, A_IDENTIFIER, parent);
    consume(status);
    return ident;
}
// @TODO : Separate variable- assignments & declarations
struct AST_NODE *parse_assignment(struct PARSER_STATUS *status, struct AST_NODE *lhs, struct AST_NODE *parent)
{
    struct AST_NODE *node = make_node(status, A_ASSIGN, parent);

    printf("Parsing Assignment Statement\n");

    // (identifier): type = value
    lhs->parent = node;
    node->lhs = lhs;

    // identifier: (type) = value : OPTIONAL // Implies a previous declaration if no type is specified
    if (consume_if(status, T_COLON) == 1)
    {
        node->vartype = parse_type(status, node);
        if (node->vartype == NULL)
        {
            print_error_exit(status->current->col, status->current->row,
                             "No type specified during variable declaration");
        }
    }

    // identifier: type (= value) : OPTIONAL // Implies no previous declaration if a value AND type is specified, if
    // it's just a value, then it's implied that a previous declaration exists.
    if (consume_if(status, T_ASSIGN) == 1)
    {
        node->rhs = parse_expression(status, node);
    }

    return node;
}
// Attempt to find either an identifier or a type-identifier pair
struct AST_NODE *parse_argument(struct PARSER_STATUS *status, struct AST_NODE *node)
{
    printf("Parsing Argument\n");

    struct AST_NODE *tmp_type, *tmp_expr;
    tmp_type = parse_type(status, node);
    tmp_expr = parse_expression(status, node);
    if (tmp_expr == NULL)
    {
        return NULL;
    }

    tmp_expr->vartype = tmp_type;
    node->args.push_back(tmp_expr);
}

struct AST_NODE *parse_statement(struct PARSER_STATUS *status, struct AST_NODE *parent)
{
    struct AST_NODE *node = NULL;

    switch (status->current->type)
    {
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
        node->body = parse_block(status, node); // parse_block() consumes both '{' and '}'

        // Handle 'else' branches for 'if' statements
        if (node->type == A_IF && status->current->type == T_ELSE)
        {
            node->els = parse_block(status, node);
        }

        return node; // Return to remove the ';' requirement for 'block' statements
    case T_LBRACE:   // Dangling block
        printf("Entering block parse\n");
        node = parse_block(status, parent);
        printf("Exiting block parse\n");
        return node; // Return to remove the ';' requirement for 'block' statements

    case T_PRINT:
        node = make_node(status, A_PRINT, parent);
        consume(status);
        printf("Parsing Print Statement\n");
        node->lhs = parse_expression(status, node);
        break;

    case T_FUNC:
        node = make_node(status, A_FUNC_DEF, parent);
        printf("Parsing Function Definition Statement\n");
        consume(status);

        // Parse Type
        node->vartype = parse_type(status, node);
        if (node->vartype == NULL)
        {
            print_error_exit(status->current->col, status->current->row, "No type specified in function definition");
        }

        // Parse Function Name
        node->name = parse_identifier(status, node);
        printf("Parsing function def with name %s\n", node->name->token->literal_value);
        if (node->name == NULL)
        {
            print_error_exit(status->current->col, status->current->row,
                             "Expected an identifier in funcion definition");
        }

        // Parse Parameter Declarations
        consume_assert(status, T_LPARENS,
                       "Expected '(' at the beginning of argument declarations in "
                       "function definition");

        struct AST_NODE *vartype;
        struct AST_NODE *arg;
        while (status->current->type != T_RPARENS)
        {
            arg = parse_identifier(status, node);
            consume(status); // Consume the T_COLON
            vartype = parse_type(status, node);

            if (arg == NULL)
            {
                print_error_exit(status->current->col, status->current->row,
                                 "Expected an identifier in argument declaration in "
                                 "function definition");
            }

            if (vartype == NULL)
            {
                print_error_exit(status->current->col, status->current->row,
                                 "Expected a type in argument declaration in "
                                 "function definition");
            }

            arg->vartype = vartype;
            node->args.push_back(arg);

            if (status->current->type == T_COMMA)
            {
                consume(status);
                continue;
            }
            break;
        }

        consume_assert(status, T_RPARENS,
                       "Expected ')' at the end of argument declarations in "
                       "function definition");

        // Function Block
        node->block = parse_block(status, node);

        return node; // Return to remove the ';' requirement for 'block' statements

    case T_RETURN:
        node = make_node(status, A_RETURN, parent);
        printf("Parsing Return Statement\n");
        consume(status);
        node->lhs = parse_expression(status, node);
        break;

    case T_USE:
        break; // @TODO : Gotta write a function that reads the input, parses all its functions, variables and Use's

    default: // Defaults to expression, returns NULL if no valid expression could be constructed
        node = parse_expression(status, parent);
        if (node == NULL)
        {
            break;
        }

        // Check for Assignment Statement
        if (node->type == A_IDENTIFIER && (status->current->type == T_COLON || status->current->type == T_ASSIGN))
        {
            node = parse_assignment(status, node, parent);
        }
        break;
    }

    // Consume the ';' at the end of the statement
    if (node != NULL)
    {
        consume_assert(status, T_SEMICOLON, "Expected a ';' at the end of the statement");
    }

    return node;
}
//@TODO : Not finished, add floats etc.
struct AST_NODE *parse_factor(struct PARSER_STATUS *status, struct AST_NODE *parent)
{
    struct AST_NODE *node;

    switch (status->current->type)
    {
    case T_LPARENS:
        consume(status);
        node = parse_expression(status, parent);
        consume_assert(status, T_RPARENS, "Expected a ')' at the end of a '(expr)' ");

        printf("parse_factor :: T_LPARENS :: %c\n", status->current->value.charval);

        return node; // we return instead of breaking as we need to perform
                     // 'consume_assert()' and breaking would only perform
                     // 'consume()'
    case T_STRING:
        node = make_node(status, A_STRING, parent);

        printf("parse_factor :: T_char * :: %s\n", status->current->value.strval);

        break;
    case T_INTEGER:
        node = make_node(status, A_INTEGER, parent);

        printf("parse_factor :: T_INTEGER :: %d\n", status->current->value.intval);

        break;
    case T_IDENTIFIER:

        // Expression is a function call
        if (status->current->next->type == T_LPARENS)
        {

            printf("parse_factor :: FUNCTION_CALL :: %s\n", status->current->literal_value);

            node = make_node(status, A_FUNC_CALL, parent);

            printf("parse_expression :: Function Call :: %s\n", status->current->literal_value);

            struct AST_NODE *vartype;
            struct AST_NODE *arg;

            consume(status); // Consume the identifier
            consume(status); // Consume the '('
            while (status->current->type != T_RPARENS)
            {
                arg = parse_expression(status, node);
                if (arg == NULL)
                {
                    print_error_exit(status->current->col, status->current->row,
                                     "Expected an expression as argument during function call");
                }

                node->args.push_back(arg);

                if (status->current->type == T_COMMA)
                {
                    consume(status);
                    continue;
                }
                break;
            }
            consume_assert(status, T_RPARENS,
                           "Expected ')' at the end of argument declarations in "
                           "function definition");
            return node; // We return here as we've already consumed the ')' and breaking would consume what could be a
                         // ';' required by check_statement()
        }
        // Expression is just a pure identifier
        else
        {

            node = make_node(status, A_IDENTIFIER, parent);

            printf("parse_factor :: T_IDENTIFIER :: %s\n", status->current->literal_value);
        }

        break;
    case T_CHARACTER:
        node = make_node(status, A_CHARACTER, parent);
        printf("parse_factor :: T_CHARACTER :: %s\n", status->current->literal_value);
        break;
    default:
        return NULL;
    }

    consume(status);
    return node;
}

struct AST_NODE *parse_term(struct PARSER_STATUS *status, struct AST_NODE *parent)
{
    struct AST_NODE *lhs = parse_factor(status, NULL);
    if (lhs == NULL)
    {
        return NULL;
    }

    struct AST_NODE *p;

    while (status->current->type == T_STAR || status->current->type == T_DIV)
    {
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

struct AST_NODE *parse_addititive(struct PARSER_STATUS *status, struct AST_NODE *parent)
{
    struct AST_NODE *lhs = parse_term(status, NULL);
    if (lhs == NULL)
    {
        return NULL;
    }

    struct AST_NODE *p;

    while (status->current->type == T_ADD || status->current->type == T_SUB)
    {
        p = make_node(status, status->current->type == T_ADD ? A_ADD : A_SUB, NULL);

        printf("parse_addititive :: %s\n", status->current->literal_value);

        consume(status);
        lhs->parent = p;
        p->lhs = lhs;
        p->rhs = parse_addititive(status, p);
        lhs = p;
    }

    lhs->parent = parent;
    return lhs;
}

struct AST_NODE *parse_relational(struct PARSER_STATUS *status, struct AST_NODE *parent)
{
    struct AST_NODE *lhs = parse_addititive(status, NULL);
    if (lhs == NULL)
    {
        return NULL;
    }

    struct AST_NODE *p;
    while (status->current->type == T_LESS || status->current->type == T_GREAT || status->current->type == T_LEQ ||
           status->current->type == T_GEQ || status->current->type == T_EQ || status->current->type == T_NEQ)
    {
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

struct AST_NODE *parse_logical_and(struct PARSER_STATUS *status, struct AST_NODE *parent)
{
    struct AST_NODE *lhs = parse_relational(status, NULL);
    if (lhs == NULL)
    {
        return NULL;
    }

    struct AST_NODE *p;

    while (status->current->type == T_AMP)
    {
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

struct AST_NODE *parse_logical_or(struct PARSER_STATUS *status, struct AST_NODE *parent)
{
    struct AST_NODE *lhs = parse_logical_and(status, NULL);
    if (lhs == NULL)
    {
        return NULL;
    }

    struct AST_NODE *p;

    while (status->current->type == T_PIPE)
    {
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

struct AST_NODE *parse_expression(struct PARSER_STATUS *status, struct AST_NODE *parent)
{
    struct AST_NODE *lhs = parse_logical_or(status, NULL);
    if (lhs == NULL)
    {
        return NULL;
    }

    struct AST_NODE *p;
    while (status->current->type == T_ASSIGN)
    {
        p = make_node(status, A_ASSIGN, NULL);

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

/* Convert the linked list of tokens into an abstract syntax tree, entrypoint
 * for the recursion */
struct AST_NODE *parse(char *input, int size, struct TOKEN *head)
{
    // Initialization
    struct PARSER_STATUS *status = (struct PARSER_STATUS *)malloc(sizeof(struct PARSER_STATUS));
    struct AST_NODE *root = (struct AST_NODE *)malloc(sizeof(struct AST_NODE));
    status->head = head;
    status->current = head;
    status->vpos = 0;
    status->input = input;
    status->input_size = size;

    printf("/* BEGIN PARSING */\n");

    status->root = parse_block(status, NULL);
    printf("/* FINISHED PARSING */\n");
    return status->root;
}
