#define CATCH_CONFIG_MAIN
#include "ext/catch.hpp"

#include "../cjson.h"

#define JSON_DUMMY_TOKEN() \
    ((json_jsontoken*) malloc(sizeof(json_jsontoken)))


/**
 * Test list operations:
 * - creating the list
 * - appending to the list
 * - lists' ability to resize automatically.
 */
TEST_CASE( "json_jsontoken_list", "[json_jsontoken_list]" )
{
    auto first = JSON_DUMMY_TOKEN();
    auto second = JSON_DUMMY_TOKEN();
    auto third = JSON_DUMMY_TOKEN();
    json_jsontoken_list *l = json_jsontoken_list_create(1);
    json_jsontoken_list_append(l, first);
    json_jsontoken_list_append(l, second);
    json_jsontoken_list_append(l, third);
    REQUIRE( l->tokens[0] == first );
    REQUIRE( l->tokens[1] == second );
    REQUIRE( l->tokens[2] == third );
    REQUIRE( l->length == 3 );
    free(first);
    free(second);
    free(third);
    free(l->tokens);
    free(l);
}


/**
 * Test creating tokens
 */
TEST_CASE( "json_jsontoken_create ", "[json_jsontoken_create]" )
{
    json_jsontoken *token = json_jsontoken_create(JSON_OBJ, NULL);
    REQUIRE( token->children->length == 0 );
    REQUIRE( token->parent == NULL );
    REQUIRE( token->type == JSON_OBJ );
    REQUIRE( token->error == false );
    free(token->children->tokens);
    free(token->children);
    free(token);
}


TEST_CASE( "json_parsenull_success", "[json_parsenull]" )
{
    char *null_str = "null ";
    json_parser *p = json_parser_create(null_str);
    REQUIRE( json_parsenull(p, p->all_tokens->tokens[0]) == true );
    json_jsontoken *null_token = p->all_tokens->tokens[1];
    REQUIRE( null_token->parent == p->all_tokens->tokens[0] );
    REQUIRE( null_token->start_in == 0 );
    REQUIRE( null_token->end_in == 4 );
    REQUIRE( null_token->type == JSON_NUL );
    json_parser_cleanup(p);
}


TEST_CASE( "json_parsenull_fail", "[json_parsenull]" )
{
    char *null_str = "nul ";
    json_parser *p = json_parser_create(null_str);
    REQUIRE( json_parsenull(p, p->all_tokens->tokens[0]) == false );
    REQUIRE( p->all_tokens->tokens[0]->error == true );
    json_parser_cleanup(p);
}


TEST_CASE( "json_parsestr_success", "[json_parsestr]" )
{
    char *str_str = "\"Valid json str\"";
    json_parser *p = json_parser_create(str_str);
    REQUIRE( json_parsestr(p, p->all_tokens->tokens[0]) == true );
    json_jsontoken *t = p->all_tokens->tokens[1];
    REQUIRE( t->start_in == 1 ); /** 'V' */
    REQUIRE( t->end_in == 15 ); /** '\"' */
    REQUIRE( t->type == JSON_STR );
    json_parser_cleanup(p);
}
