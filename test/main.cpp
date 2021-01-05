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

TEST_CASE( "json_jsontoken_create ", "[json_jsontoken_create" )
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
