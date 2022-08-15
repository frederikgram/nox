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

// Certain AST_NODEs such as the literal token 'int' or 'str' needs to be converted from a NODE type to a VARIABLE type 'integer' or 'string'
struct VARIABLE_TYPE *get_type_from_subtree(struct AST_NODE *node)
{
    struct VARIABLE_TYPE *type = new struct VARIABLE_TYPE;

    switch (node->type)
    {
    case A_ARRAY:
    case A_ARR:
        type->type = V_ARRAY;
        type->array_type = get_type_from_subtree(node->vartype);
        return type;
    case A_INTEGER:
    case A_INT:
        type->type = V_INTEGER;
        return type;
    case A_CHARACTER:
    case A_CHAR:
        type->type = V_CHARACTER;
        return type;
    case A_STRING:
    case A_STR:
        type->type = V_STRING;
        return type;
    case A_VOID:
        type->type = V_VOID;
        return type;
    default: 
        fprintf(stderr, "Unknown type at get_type_from_subtree in typecheck.cpp :: received token '%s'\n", node->token->literal_value.c_str());
        exit(-1);
    }

}
int are_arithmetiically_compatible(enum AST_NODE_TYPE type, struct VARIABLE_TYPE *type1, struct VARIABLE_TYPE *type2)
{
  
    switch(type) {
    case A_ADD:
        if(type1->type == V_INTEGER && type2->type == V_INTEGER) {
            return 1;
        }
        if(type1->type == V_STRING && type2->type == V_STRING) {
            return 1;
        }
        return 0;
    case A_SUB:
    case A_MUL:
        if(type1->type == V_INTEGER && type2->type == V_INTEGER) {
            return 1;
        }
    case A_MOD:
        if(type1->type == V_INTEGER && type2->type == V_INTEGER) {
            return 1;
        }
        return 0;
    case A_DIV:
        if(type1->type == V_INTEGER && type2->type == V_INTEGER) {
            return 1;
        }
        return 0;

    default:
        fprintf(stderr, "Unknown type at are_arithmetiically_compatible in typecheck.cpp\n");
        exit(-1);
    }

}
int are_relationally_comparable(struct VARIABLE_TYPE *type1, struct VARIABLE_TYPE *type2)
{
    if (type1->type == V_INTEGER && type2->type == V_INTEGER)
        return 1;
    else if (type1->type == V_CHARACTER && type2->type == V_CHARACTER)
        return 1;
    else if (type1->type == V_STRING && type2->type == V_STRING)
        return 1;
    else if((type1->type == V_INTEGER && type2->type == V_CHARACTER) || (type1->type == V_CHARACTER && type2->type == V_INTEGER))
        return 1;
    else
        return 0;
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
int is_literal(struct AST_NODE *node)
{
    return node->type == A_INTEGER || node->type == A_STRING || node->type == A_CHARACTER;
}

// This function is used to check if a variable is declared in the current scope
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

struct VARIABLE_TYPE *check_expression(struct AST_NODE *expr)
{
    struct VARIABLE_TYPE *type = new struct VARIABLE_TYPE;
    printf("Typechecking\t::\tcheck_expression\t::\tChecking Expression\n");

    switch (expr->type)
    {
    case A_ARRAY: {
        type->type = V_ARRAY;
        type->array_type = check_expression(expr->vartype);
        type->array_size = expr->args.size();
        printf("Typechecking\t::\tcheck_expression\t::\tArray of type %s with size %d\n", variable_type_to_string(type->array_type), type->array_size);
        return type;
    }

    case A_INTEGER:
    case A_STRING:
    case A_VOID:
    case A_CHARACTER:
        printf("Typechecking\t::\tcheck_expression\t::\tExpression is a literal of type %s\n",
               ast_node_type_to_string(expr->type));

        return get_type_from_subtree(expr);

    case A_FUNC_CALL: {
        printf("Typechecking\t::\tcheck_expression\t::\tExpression is a function call\n");
        struct VARIABLE *func = find_variable(expr->token->literal_value);
        if (func == NULL)
        {
            fprintf(stderr,"Typechecking\t::\tcheck_expression\t::\tFunction %s not found\n", expr->token->literal_value.c_str());
            exit(-1);
        }
        printf("Typechecking\t::\tcheck_expression\t::\tFunction '%s' found\n", expr->token->literal_value.c_str());
        return func->type->return_type;
    }

    case A_ADD:
    case A_SUB:
    case A_MUL:
    case A_DIV: // @TODO : A_DIV should always return a FLOAT type
    case A_MOD: {
        printf("Typechecking\t::\tcheck_expression\t::\tExpression is a binary operation %s\n", ast_node_type_to_string(expr->type));
        
        struct VARIABLE_TYPE * lhs = check_expression(expr->lhs);
        struct VARIABLE_TYPE * rhs = check_expression(expr->rhs);

        if(are_arithmetiically_compatible(expr->type, lhs, rhs) == 0) {
            fprintf(stderr,"Typechecking\t::\tcheck_expression\t::\tExpression is not arithmetically compatible\n");
            exit(-1);
        }

        return lhs;
    }

    case A_GREAT:
    case A_LESS:
    case A_GEQ:
    case A_LEQ: {
        struct VARIABLE_TYPE * lhs = check_expression(expr->lhs);
        struct VARIABLE_TYPE * rhs = check_expression(expr->rhs);

        if(are_relationally_comparable(lhs, rhs) == 0) {
            fprintf(stderr, "Typechecking\t::\tcheck_expression\t::\tExpression is not relationally comparable\n");
            exit(-1);
        }

        type->type = V_INTEGER;
        return type;
    }

    case A_NEQ:
    case A_EQ: {
        struct VARIABLE_TYPE * lhs = check_expression(expr->lhs);
        struct VARIABLE_TYPE * rhs = check_expression(expr->rhs);

        if(are_equivalent_types(lhs, rhs) == 0) {
            fprintf(stderr,"Typechecking\t::\tcheck_expression\t::\tExpression is not equality comparable\n");
            exit(-1);
        }

        return lhs;
    }

    // A_ADDRESSOF
    // A_DEREF

    case A_IDENTIFIER: {
        printf("Typechecking\t::\tcheck_expression\t::\tExpression is an identifier\n");
        struct VARIABLE *var = find_variable(expr->token->literal_value);
        if (var == NULL)
        {
            fprintf(stderr, "Typechecking\t::\tcheck_expression\t::\tVariable %s not found\n", expr->token->literal_value.c_str());
            exit(-1);
        }
        printf("Typechecking\t::\tcheck_expression\t::\tVariable '%s' found\n", expr->token->literal_value.c_str());
        return var->type;
    }

    case A_INDEX: {
        printf("Typechecking\t::\tcheck_expression\t::\tExpression is an index\n");
        struct VARIABLE_TYPE *array_type = check_expression(expr->lhs);
        struct VARIABLE_TYPE *index_type = check_expression(expr->rhs);

        if (array_type->type != V_ARRAY)
        {
            fprintf(stderr, "Typechecking\t::\tcheck_expression\t::\tExpression is not an array\n");
            exit(-1);
        }

        if (index_type->type != V_INTEGER)
        {
            fprintf(stderr, "Typechecking\t::\tcheck_expression\t::\tExpression is not an integer\n");
            exit(-1);
        }

        return array_type->array_type;
}
    default:
        fprintf(stderr, "Typechecking\t::\tcheck_expression\t::\tUnknown expression type %s\n", ast_node_type_to_string(expr->type));
        exit(-1);
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

    case A_WHILE:
    case A_IF: {
        // Ensure that the conditional expression is of type integer
        if (check_expression(statement->conditional)->type != V_INTEGER)
        {
            fprintf(stderr,"Typechecking\t::\tcheck_statement\t::\tCondition of if statement is not an integer\n");
            exit(-1);
        }

        // Check if all statements inside the block are valid, we don't use 'check_block' as it creates a new scope
        for (auto stmt : statement->statements)
        {
            check_statement(stmt);
        }

        // If there is an else statement, ensure all its statements are valid.
        if(statement->els != NULL) {
            for (auto stmt : statement->els->statements)
            {
                check_statement(stmt);
            }
        }

        break;
    }
    case A_FUNC_DEF: {

        // Build Function Variable
        struct VARIABLE *function = new struct VARIABLE;
        function->name = statement->name->token->literal_value;

        // Fetch information type of function
        struct VARIABLE_TYPE *function_type = new struct VARIABLE_TYPE;
        function_type->type = V_FUNCTION;
        function_type->return_type = get_type_from_subtree(statement->vartype);
        function->type = function_type;

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
            fprintf(stderr,
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
            fprintf(stderr,"Typechecking\t::\tcheck_statement\t::\tReturn type does not match function return type\n");
        }

        break;
    }

    case A_PRINT:
        if (check_expression(statement->lhs)->type != V_STRING)
        {
            fprintf(stderr,"Typechecking\t::\tcheck_statement\t::\tPrint statement is not a string\n");
            exit(-1);
        }
        break;

    case A_ASSIGN: {
        printf("Typechecking\t::\tcheck_statement\t::\tChecking Assignment\n");

        // Check that the variable exists
        struct VARIABLE * lhs = find_variable(statement->lhs->token->literal_value);
        if(lhs == NULL)
        {
            if(statement->vartype == NULL) {
            printf("Typechecking\t::\tcheck_statement\t::\tVariable %s not found\n", statement->lhs->token->literal_value.c_str());
            } else {
                // Create new variable
                lhs = new struct VARIABLE;
                lhs->name = statement->lhs->token->literal_value;
                lhs->type = get_type_from_subtree(statement->vartype);
                scopes.back()->variables[lhs->name] = lhs;
            }
        }
        // Statement was purely a variable declaration
        if(statement->rhs == NULL) { break; }

        // Statement attempts to assign a value to a variable
        // Get type of RHS expression
        struct VARIABLE_TYPE *rhs = check_expression(statement->rhs);

        // If the types are not equivalent, throw an error
        if(are_equivalent_types(lhs->type, rhs) == 0)
        {
            fprintf(stderr,"Typechecking\t::\tcheck_statement\t::\tVariable '%s' is of type %s, cannot assign value of type %s\n", lhs->name.c_str(), variable_type_to_string(lhs->type), variable_type_to_string(rhs));
            exit(-1);
        }
        break;
    }

    // @TODO : Input statement

    default: check_expression(statement);break;

    }
}

// Checks a block of statements, creating a new scope if necessary
void check_block(struct AST_NODE *block, std::vector<struct VARIABLE *> params)
{


    printf("Typechecking\t::\tcheck_block\t::\tChecking block\n");

    push_scope();
    block->scope = scopes.back();
    if(scopes.size() > 1) {
        scopes.back()->parent = scopes[scopes.size() - 2];
    } else {
        scopes.back()->parent = NULL;
    }

    if (params.size() > 0)
    {

        printf("Typechecking\t::\tcheck_block\t::\tInjecting %d variables into scope\n", params.size());

        for (auto *param : params)
        {
            scopes.back()->variables[param->name] = param;
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