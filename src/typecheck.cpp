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

struct VARIABLE *find_variable(struct SCOPE *scope, std::string name)
{

    for (int i = scopes.size() - 1; i >= 0; i--)
    {
        if (auto var = scopes[i]->variables.find(name); var != scopes[i]->variables.end())
        {
            return var->second;
        }
    }

    return NULL;
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
    case A_ARRAY:
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

    return (a->type == b->type) && (are_equivalent_types(a->array_type, b->array_type)) &&
           (are_equivalent_types(a->pointer_to, b->pointer_to));
}

struct VARIABLE_TYPE *check_expression(struct AST_NODE *expr)
{

    if (expr == NULL)
    {
        return NULL;
    }

    struct VARIABLE_TYPE *type = new struct VARIABLE_TYPE;

#ifdef DEBUG
    printf("Typechecking\t::\tcheck_expression\t::\tChecking Expression\n");
#endif

    switch (expr->type)
    {
    case A_INTEGER:
    case A_STRING:
    case A_CHARACTER:
    case A_ARRAY:
#ifdef DEBUG
        printf("Typechecking\t::\tcheck_expression\t::\tExpression is a literal of type %s\n",
               ast_node_type_to_string(expr->type));
#endif
        return get_type_from_subtree(expr);
    }
}

void check_statement(struct AST_NODE *statement)
{

#ifdef DEBUG
    printf("Typechecking\t::\tcheck_statement\t::\tChecking statement of type %s\n",
           ast_node_type_to_string(statement->type));
#endif

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
#ifdef DEBUG
        printf("Typechecking\t::\tcheck_statement\t::\tFunction return type is %s\n",
               variable_type_to_string(function_type->return_type));
#endif

        // Add parameters to function
        struct VARIABLE *tmp;
        for (auto arg : statement->args)
        {
            tmp = new struct VARIABLE;
            tmp->type = get_type_from_subtree(arg->vartype);
            tmp->name = arg->token->literal_value;
            function->parameters.push_back(tmp);
#ifdef DEBUG

            printf("Typechecking\t::\tcheck_statement\t::\tFunction arg type is %s\n",
                   variable_type_to_string(tmp->type));
#endif
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
#ifdef DEBUG

        printf("Typechecking\t::\tcheck_statement\t::\tChecking return statement\n");
#endif

        if (functions.size() == 0)
        {
            print_error_exit(statement->token->col, statement->token->row,
                             "Typechecking\t::\tcheck_statement\t::\tReturn statement outside of function\n");
        }
#ifdef DEBUG

        printf("Typechecking\t::\tcheck_statement\t::\tExpecting Return type %s\n",
               variable_type_to_string(functions.back()->type->return_type));
#endif

        struct VARIABLE_TYPE *return_type = check_expression(statement->lhs);
#ifdef DEBUG

        printf("Typechecking\t::\tcheck_statement\t::\tReceived Return type %s\n",
               variable_type_to_string(return_type));
#endif

        if (are_equivalent_types(functions.back()->type->return_type, return_type) == 0)
        {
            print_error_exit(
                statement->token->col, statement->token->row,
                "Typechecking\t::\tcheck_statement\t::\tReturn type does not match function return type\n");
        }
    }
    }
}

void check_block(struct AST_NODE *block, std::vector<struct VARIABLE *> params)
{
#ifdef DEBUG

    printf("Typechecking\t::\tcheck_block\t::\tChecking block\n");
#endif

    push_scope();

    if (params.size() > 0)
    {
#ifdef DEBUG
        printf("Typechecking\t::\tcheck_block\t::\tInjecting %d variables into scope\n", params.size());
#endif
        for (auto *param : params)
        {

            // push_variable(scopes.back(), param->name, NULL, param->value);
        }
    }

    for (auto statement : block->statements)
    {
        check_statement(statement);
    }

    pop_scope();
#ifdef DEBUG
    printf("Typechecking\t::\tcheck_block\t::\tExiting block\n");
#endif
}

struct AST_NODE *typecheck(struct AST_NODE *root)
{
    check_block(root, {});
}