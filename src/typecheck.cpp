/**/

#include "typecheck.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "utils.hpp"
#include <assert.h>
#include <iostream>
#include <math.h>
#include <ostream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
using namespace std;

vector<struct SCOPE *> scopes;
vector<struct VARIABLE *> functions;

int scope_id = 0;

void push_scope()
{
    struct SCOPE *scope = new struct SCOPE;
    scope->id = scope_id++;
    scopes.push_back(scope);
}

struct SCOPE *pop_scope()
{
    struct SCOPE *scope = scopes.back();
    scopes.pop_back();
    return scope;
}

struct VARIABLE_TYPE *get_type_from_subtree(struct AST_NODE *node)
{

    if (node == NULL)
    {
        return NULL;
    }

    struct VARIABLE_TYPE *type = new struct VARIABLE_TYPE;

    switch (node->type)
    {
    case A_ARR:
        type->type = V_ARRAY;
        type->array_type = get_type_from_subtree(node->vartype);
        break;
    case A_INT:
        type->type = V_INTEGER;
        break;
    case A_CHAR:
        type->type = V_CHARACTER;
        break;
    case A_STRING:
        type->type = V_STRING;
        break;
    default:
        type->type = ast_type_to_variable_type_enum(node->type);
        break;
    }

    return type;
}

int are_equivalent_types(struct VARIABLE_TYPE *a, struct VARIABLE_TYPE *b)
{
    if (a == NULL && b == NULL)
    {
        return 1;
    }
    else if (a == NULL || b == NULL)
    {
        return 0;
    }
    printf("Typechecking\t::\tget_type_from_subtree\t::\tComparing types %s and %s\n", variable_type_to_string(a), variable_type_to_string(b));

    return (a->type == b->type) && (are_equivalent_types(a->array_type, b->array_type)) &&
           (are_equivalent_types(a->pointer_to, b->pointer_to));
}

struct VARIABLE * find_variable(std::string name)
{
    for (int i = scopes.size() - 1; i >= 0; i--)
    {
        struct SCOPE *scope = scopes[i];
        if (scope->variables.find(name) != scope->variables.end())
        {
            return scope->variables[name];
        }
    }
    return NULL;
}

// @TODO : Placeholder logic for now
struct VARIABLE_TYPE *check_expression(struct AST_NODE *expr)
{

    if (expr == NULL)
    {
        return NULL;
    }

    struct VARIABLE_TYPE *type = new struct VARIABLE_TYPE;


    printf("Typechecking\t::\tcheck_expression\t::\tChecking Expression\n");


    switch (expr->type)
    {
    case A_ARRAY: {
        struct VARIABLE_TYPE *type = new struct VARIABLE_TYPE;
        type->type = V_ARRAY;
        type->array_type = check_expression(expr->vartype);
        type->array_size = expr->args.size();
        printf("Typechecking\t::\tcheck_expression\t::\tArray of type %s with size %d\n", variable_type_to_string(type->array_type), type->array_size);
        return type;
    }
    case A_INTEGER:
    case A_STRING:
    case A_CHARACTER:

        printf("Typechecking\t::\tcheck_expression\t::\tExpression is a literal of type %s\n",
               ast_node_type_to_string(expr->type));

        return get_type_from_subtree(expr);
    default:break;
    }
}

void check_statement(struct AST_NODE *statement)
{

    printf("Typechecking\t::\tcheck_statement\t::\tChecking statement of type %s\n",
           ast_node_type_to_string(statement->type));

    switch (statement->type)
    {

    case A_BLOCK:
        check_block(statement, {});
        break;

    case A_FUNC_DEF: {

        // Build Function Variable
        struct VARIABLE *function = new struct VARIABLE;
        function->name = statement->name->token->literal_value;

        // Fetch information type of function
        struct VARIABLE_TYPE *function_type = new struct VARIABLE_TYPE;
        function->type = function_type;

        function_type->type = V_FUNCTION;
        function_type->return_type = get_type_from_subtree(statement->vartype);

        printf("Typechecking\t::\tcheck_statement\t::\tFunction return type is %s\n",
               variable_type_to_string(function_type->return_type));


        // Add parameters to function
        struct VARIABLE *tmp;
        for (auto arg : statement->args)
        {
            tmp = new struct VARIABLE;
            tmp->type = get_type_from_subtree(arg->vartype);
            tmp->name = arg->token->literal_value;
            function->parameters.push_back(tmp);

            printf("Typechecking\t::\tcheck_statement\t::\tFunction arg type is %s\n",
                   variable_type_to_string(tmp->type));

        }

        // Add function to scope
        scopes.back()->variables[function->name] = function;

        // Check function body & Update function stack
        functions.push_back(function);
        check_block(statement->block, function->parameters);
        functions.pop_back();
        break;
    }

    case A_RETURN: {

        printf("Typechecking\t::\tcheck_statement\t::\tChecking return statement\n");

        // Ensure that the return statement is inside a function
        if (functions.size() == 0)
        {
            print_error_exit(statement->token->col, statement->token->row,
                             "Typechecking\t::\tcheck_statement\t::\tReturn statement outside of function\n");
        }

        printf("Typechecking\t::\tcheck_statement\t::\tExpecting Return type %s\n",
               variable_type_to_string(functions.back()->type->return_type));

        // Get the return type of the expression we are returning
        struct VARIABLE_TYPE *return_type = check_expression(statement->lhs);

        printf("Typechecking\t::\tcheck_statement\t::\tReceived Return type %s\n",
               variable_type_to_string(return_type));

        // Ensure that the return type matches the function return type
        if (are_equivalent_types(functions.back()->type->return_type, return_type) == 0)
        {
            print_error_exit(
                statement->token->col, statement->token->row,
                "Typechecking\t::\tcheck_statement\t::\tReturn type does not match function return type\n");
        }
    }

    // Binary Expression (Non-Relational)
    case A_ADD:
    case A_SUB:
    case A_MUL:
    case A_DIV:
    case A_LAND:
    case A_LOR:
        printf("Typechecking\t::\tcheck_statement\t::\tChecking binary expression\n");
        break;

    case A_LESS:
    case A_GREAT:
    case A_LEQ:
    case A_GEQ:
    case A_EQ:
    case A_NEQ:
        printf("Typechecking\t::\tcheck_statement\t::\tChecking relational expression\n");
        break;

    case A_ASSIGN: {
        printf("Typechecking\t::\tcheck_statement\t::\tChecking Assignment\n");

        // Check that the variable exists
        struct VARIABLE * lhs = find_variable(statement->lhs->token->literal_value);
        if(lhs == NULL)
        {
            if(statement->vartype == NULL) {
            print_error_exit(statement->token->col, statement->token->row,
                             "Typechecking\t::\tcheck_statement\t::\tVariable %s not found\n",
                             statement->lhs->token->literal_value);
            } else {
                // Create new variable
                lhs = new struct VARIABLE;
                lhs->name = statement->lhs->token->literal_value;
                lhs->type = get_type_from_subtree(statement->vartype);
                scopes.back()->variables[lhs->name] = lhs;
            }
        }

        // Statement was purely a variable declaration
        if(statement->rhs == NULL) {
            break;
        }
        // Statement attempts to assign a value to a variable

        // Get type of RHS expression
        struct VARIABLE_TYPE *rhs = check_expression(statement->rhs);
        if(rhs == NULL) {
            printf("eeeeeeeeeeeeee\n");
        }

        // If the types are not equivalent, throw an error
        if(are_equivalent_types(lhs->type, rhs) == 0)
        {
            printf("Typechecking\t::\tcheck_statement\t::\tVariable '%s' is of type %s, cannot assign value of type %s\n", lhs->name.c_str(), variable_type_to_string(lhs->type), variable_type_to_string(rhs));
            exit(-1);
        }


        break;
    }}
}

void check_block(struct AST_NODE *block, std::vector<struct VARIABLE *> params)
{


    printf("Typechecking\t::\tcheck_block\t::\tChecking block\n");


    push_scope();

    if (params.size() > 0)
    {

        printf("Typechecking\t::\tcheck_block\t::\tInjecting %d variables into scope\n", params.size());

        for (auto *param : params)
        {

            // @TODO : push_variable(scopes.back(), param->name, NULL, param->value);
        }
    }

    for (auto statement : block->statements)
    {
        check_statement(statement);
    }

    pop_scope();

    printf("Typechecking\t::\tcheck_block\t::\tExiting block\n");

}

struct AST_NODE *typecheck(struct AST_NODE *root)
{
    check_block(root, {});
}