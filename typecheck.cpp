/**/

#include "typecheck.hpp"
#include "lexer.hpp"
#include "parser.hpp"
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

void push_scope()
{
    struct SCOPE *scope = new struct SCOPE;
    scopes.push_back(scope);
}

struct SCOPE *pop_scope()
{
    struct SCOPE *scope = scopes.back();
    scopes.pop_back();
    return scope;
}

enum VARIABLE_TYPE type_specifier_to_variable_type(enum AST_NODE_TYPE type)
{
    switch (type)
    {
    case A_INT:
        return V_INTEGER;
    case A_CHAR:
        return V_CHARARACTER;
    case A_STRING:
        return V_STRING;
    default:
        return V_UNKNOWN;
    }
}

enum VARIABLE_TYPE ast_type_to_variable_type(enum AST_NODE_TYPE type)
{
    switch (type)
    {
    case A_INTEGER:
        return V_INTEGER;
    case A_CHARACTER:
        return V_CHARARACTER;
    case A_STRING:
        return V_STRING;
    case A_IDENTIFIER:
        return V_IDENTIFIER;
    case A_FUNC_DEF:
    case A_FUNC_CALL:
        return V_FUNCTION;
    default:
        return V_UNKNOWN;
    }
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

struct VARIABLE *push_variable(struct SCOPE *scope, std::string name, enum VARIABLE_TYPE type, void *value)
{

    if (scope->variables.count(name) > 0)
    {
        return NULL;
    }

    printf("Typechecking :: push_variable :: Creating new Variable with name %s\n", name);

    struct VARIABLE *variable = new struct VARIABLE;
    variable->name = name;
    variable->type = type;
    variable->value = value;
    scope->variables[name] = variable;

    return variable;
}

enum VARIABLE_TYPE check_expression(struct AST_NODE *expr)
{
    struct VARIABLE *var;

    struct VARIABLE *lhs_var;
    struct VARIABLE *rhs_var;
    enum VARIABLE_TYPE var_type;
    enum VARIABLE_TYPE lhs_type;
    enum VARIABLE_TYPE rhs_type;
    std::string var_name;

    switch (expr->type)
    {

    // Binary Expressions
    case A_ADD:
    case A_SUB:
    case A_MUL:
    case A_DIV:
    case A_EQ:
    case A_NEQ:
    case A_LESS:
    case A_GREAT:
    case A_LEQ:
    case A_GEQ:
    case A_LAND:
    case A_LOR:
    case A_AND:
    case A_OR:

        if (expr->lhs == NULL || expr->rhs == NULL)
        {
            printf("Typechecking :: check_expression :: Binary Expression with NULL LHS or RHS\n");
            exit(1);
        }

        lhs_type = check_expression(expr->lhs);
        rhs_type = check_expression(expr->rhs);

        // Check lhs and rhs type compatability - @TODO : This assumes type symmetry for binary expressions, this should
        // be changed probably
        if (lhs_type != rhs_type)
        {
            printf("Typechecking :: check_expression :: Binary Expression with LHS and RHS of different types\n");
            exit(1);
        }

        return lhs_type;
        break;

    // Unary Expressions @TODO : Not yet implemented
    case A_NOT:
        break;

    // @TODO : Check function call
    case A_FUNC_CALL:

        // Check if function exists
        var_name = expr->lhs->token->literal_value;
        if (auto func = find_variable(scopes.back(), var_name); func == NULL || func->type != V_FUNCTION)
        {
            printf("Typechecking :: check_expression :: Function call to undefined function %s");
            exit(1);
        }

        break;

    case A_IDENTIFIER:
        var_name = expr->token->literal_value;
        printf("Typechecking :: check_expression :: Looking for Variable with name '%s'\n", expr->token->literal_value);
        var = find_variable(scopes.back(), var_name);
        if (var == NULL)
        {
            printf("Typechecking :: check_expression :: Variable %s not found\n", expr->token->literal_value);
            exit(1);
        }
        return var->type;

    // Currently this attempts to handle literals such as A_INTEGER etc.
    default:
        return ast_type_to_variable_type(expr->type);
    }
}

void check_statement(struct AST_NODE *statement)
{

    printf("Typechecking :: Checking Statement\n");
    struct VARIABLE *tmp;
    std::string name;
    enum VARIABLE_TYPE tmpt;

    switch (statement->type)
    {

    // Function Definition
    case A_FUNC_DEF:
        name = statement->name->token->literal_value;
        // Check if function already exists
        if (auto func = find_variable(scopes.back(), name); func != NULL)
        {
            printf("Typechecking :: check_statement :: Function %s already defined\n", name);
            exit(1);
        }

        // Push function to scope
        push_variable(scopes.back(), name, V_FUNCTION, NULL);

        // Check function parameters

        // Check function body
        check_function_block(statement);
        break;

    case A_WHILE:
    case A_IF:
        tmpt = check_expression(statement->conditional);
        if (tmpt != V_INTEGER)
        {
            printf("Typechecking :: Error :: Expected Integer in Conditional Statement\n");
            exit(1);
        }

        check_block(statement->body);
        break;

    case A_ASSIGN:
        // Variable Declaration
        if (statement->vartype != NULL)
        {
            std::string name = statement->lhs->token->literal_value;
            // When declaring a variable it should not already exist in the current scope.
            if (find_variable(scopes.back(), name) != NULL)
            {
                printf("Typechecking :: check_statement :: Variable with name '%s' has already been declared in "
                       "current "
                       "scope\n",
                       statement->lhs->token->literal_value);
                exit(1);
            }
            // This is a variable declaration, so we want to push it to the current scope.
            printf("Pushing var with name '%s'\n", statement->lhs->token->literal_value);
            tmp = push_variable(scopes.back(), name, type_specifier_to_variable_type(statement->vartype->type), NULL);
        }
        // This is a variable assignment, so we want to find it in the current scope.
        else
        {
            printf("Attempting to find var with name '%s'\n", statement->lhs->token->literal_value);
            std::string name = statement->lhs->token->literal_value;
            tmp = find_variable(scopes.back(), name);
            if (tmp == NULL)
            {
                printf("Typechecking :: check_statement :: Variable '%s' does not exist in current scope\n",
                       statement->lhs->token->literal_value);
                exit(1);
            }
        }
        // The variable is getting a value assigned to it.
        if (statement->rhs != NULL)
        {
            // @TODO : This is a place where it could be interesting to use some Coercion helper methods if we
            // wanted to support implicit casting from e.g. int to float
            enum VARIABLE_TYPE rhs_type = check_expression(statement->rhs);
            enum VARIABLE_TYPE expected_type =
                statement->vartype == NULL ? tmp->type : type_specifier_to_variable_type(statement->vartype->type);
            if (expected_type != rhs_type)
            {
                printf("Error: Variable '%s' is assigned a value of incorrect type \n",
                       statement->lhs->token->literal_value);
                exit(1);
            }
            // Actual assignment
            tmp->value = (void *)statement->rhs->token->literal_value;
        }
        break;

    default:
        break;
    }
}
void print_variables_in_scope(struct SCOPE *scope)
{
    printf("Printing variables in current scope\n");
    for (auto var : scope->variables)
    {
        std::cout << " | " << var.second->name << " " << var.second->type << " " << std::endl;
    }
}

// Checks a functions block and injects its parameters into the new scope @TODO : this could just be an injection into
// the regular check_block
void check_function_block(struct AST_NODE *function_node)
{

    printf("Typechecking :: checking function block\n");

    push_scope();

    std::string name;
    // Inject parameters inside new function scope
    for (auto param : function_node->args)
    {
        name = param->token->literal_value;
        push_variable(scopes.back(), name, type_specifier_to_variable_type(param->vartype->type), NULL);
    }

    print_variables_in_scope(scopes.back());

    for (int i = 0; i < function_node->statements.size(); i++)
    {
        check_statement(function_node->statements[i]);
    }
    pop_scope();
}

void check_block(struct AST_NODE *block)
{
    printf("Typechecking :: checking block\n");

    push_scope();
    for (int i = 0; i < block->statements.size(); i++)
    {
        check_statement(block->statements[i]);
    }
    pop_scope();
}

struct AST_NODE *typecheck(struct AST_NODE *root)
{
    for (int i = 0; i < root->statements.size(); i++)
    {
        printf("%s\n", root->statements[i]->token->literal_value);
    }

    check_block(root);
}