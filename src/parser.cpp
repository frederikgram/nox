/**/

#include "parser.hpp"
#include "lexer.hpp"
#include "utils.hpp"
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

// Exit with flag -1 if status->current->type != the given TOKEN_TYPE type
void consume_assert(struct PARSER_STATUS *status, enum TOKEN_TYPE type, const char *format, ...)
{
    if (consume_if(status, type) == 0)
    {
        va_list lst;
        va_start(lst, format);
        print_error(status->current->col, status->current->row, format, lst);
        va_end(lst);
        exit(-1);
    }
}

struct AST_NODE * parse_program(struct PARSER_STATUS *status, struct AST_NODE *parent)
{
    struct AST_NODE *node = make_node(status, A_BLOCK, parent);
    struct AST_NODE *stmt;
    while ((stmt = parse_statement(status, node)) != NULL)
    {
        node->statements.push_back(stmt);
    };

    return node;
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

    printf("Parsing\t::\tparse_type\t::\tParsing Type\n");

    struct AST_NODE *type_node;
    struct AST_NODE *array_node;
 


    if(status->current->type ==  T_INT || status->current->type == T_STR || status->current->type == T_CHAR || status->current->type == T_VOID)
    {
        type_node = make_node(status, token_type_to_node_type(status->current->type), parent);
        consume(status);

        if(status->current->type == T_VOID) { return type_node;}

        // Parse Array Types
        while(status->current->type == T_LBRACKET) {

            consume(status);
            consume_assert(status, T_RBRACKET, "Expected ']' but received '%s'", status->current->literal_value);

            printf("Parsing\t::parse_factor\t::\tFound array type\n");

            array_node = make_node(status, A_ARR, parent);
            type_node->parent = array_node;
            array_node->vartype = type_node;
            array_node->parent = parent;
            type_node = array_node;
        }
    } else {
        print_error(status->current->col, status->current->row, "Expected type but received '%s'", status->current->literal_value);
        exit(-1);
    }

    return type_node;
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
        } else if (node->vartype->type == A_VOID) {
            print_error_exit(status->current->col, status->current->row,
                             "Cannot declare a variable of type 'void'");
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

        printf("Parsing\t::\tparse_statement\t::\tParsing Return Statement\n");

        consume(status);
        node->lhs = parse_expression(status, node);
        break;

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
//@TODO : Not finished, add floats etc. ADD ARRAY LITERALS
struct AST_NODE *parse_factor(struct PARSER_STATUS *status, struct AST_NODE *parent)
{

    struct AST_NODE *node;

    switch (status->current->type)
    {
    // Expression is a (expr)
    case T_LPARENS:
        consume(status);
        node = parse_expression(status, parent);
        consume_assert(status, T_RPARENS, "Expected a ')' at the end of a '(expr)' ");


        printf("parse_factor\t::\tT_LPARENS\t::\t%c\n", status->current->value.charval);


        return node; // we return instead of breaking as we need to perform
                     // 'consume_assert()' and breaking would only perform
                     // 'consume()'

    // Expression is an array literal
    case T_LBRACKET:
        consume(status);
        node = make_node(status, A_ARRAY, parent);
        while (status->current->type != T_RBRACKET)
        {
            node->args.push_back(parse_expression(status, node));
            if (status->current->type == T_COMMA)
            {
                consume(status);
                continue;
            }
            break;
        }
        consume_assert(status, T_RBRACKET, "Expected a ']' at the end of an array literal");
        node->vartype = node->args[0]; // @TODO : This seems weird, but it works...
        return node;

    case T_STRING:
        node = make_node(status, A_STRING, parent);
        printf("parse_factor\t::\tT_char *\t::\t%s\n", status->current->value.strval);
        break;
    case T_INTEGER:
        node = make_node(status, A_INTEGER, parent);
        printf("parse_factor\t::\tT_INTEGER\t::\t%d\n", status->current->value.intval);
        break;
    case T_IDENTIFIER:

        // Expression is a function call
        if (status->current->next->type == T_LPARENS)
        {
            printf("parse_factor\t::\tFUNCTION_CALL\t::\t%s\n", status->current->literal_value);
            node = make_node(status, A_FUNC_CALL, parent);
            printf("parse_expression\t::\tFunction Call\t::\t%s\n", status->current->literal_value);

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
            printf("parse_factor\t::\tT_IDENTIFIER\t::\t%s\n", status->current->literal_value);
        }

        break;
    case T_CHARACTER:
        node = make_node(status, A_CHARACTER, parent);
        printf("parse_factor\t::\tT_CHARACTER\t::\t%s\n", status->current->literal_value);
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
    if (lhs == NULL){return NULL;}

    struct AST_NODE *p;

    while (status->current->type == T_STAR || status->current->type == T_DIV || status->current->type == T_MOD)
    {
        p = make_node(status, token_type_to_node_type(status->current->type), NULL);

        printf("parse_term\t::\t%s\n", status->current->literal_value);

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


        printf("parse_addititive\t::\t%s\n", status->current->literal_value);


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


        printf("parse_relational\t::\t%s\n", status->current->literal_value);


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


        printf("parse_logical_and\t::\t%s\n", status->current->literal_value);


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


        printf("parse_logical_or\t::\t%s\n", status->current->literal_value);

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


        printf("parse_expression\t::\t%s\n", status->current->literal_value);


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

    status->root = parse_program(status, NULL);
    return status->root;
}
