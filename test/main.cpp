#define CATCH_CONFIG_MAIN
#include "extern/catch.hpp"

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


TEST_CASE( "json_parsenum_int", "[json_parsenum]" )
{
    char *int_str = "-12345 ";
    json_parser *p = json_parser_create(int_str);
    REQUIRE( json_parsenum(p, p->all_tokens->tokens[0]) == true );
    json_jsontoken *t = p->all_tokens->tokens[1];
    REQUIRE( t->start_in == 0 );
    REQUIRE( t->end_in == 6 );
    REQUIRE( t->type == JSON_INT );
    json_parser_cleanup(p);
}


TEST_CASE( "json_parsenum_float", "[json_parsenum]" )
{
    char *flo_str = "1245690123.13456E100 ";
    json_parser *p = json_parser_create(flo_str);
    REQUIRE( json_parsenum(p, p->all_tokens->tokens[0]) == true );
    json_jsontoken *t = p->all_tokens->tokens[1];
    REQUIRE( t->type == JSON_FLO );
    REQUIRE( t->start_in == 0 );
    REQUIRE( t->end_in == 20 );
    json_parser_cleanup(p);
}

TEST_CASE( "json_parsenum_error_multiple_neg", "[json_parsenum]" )
{
    char *err_str = "-123-45e1 ";
    json_parser *p = json_parser_create(err_str);
    REQUIRE( json_parsenum(p, p->all_tokens->tokens[0]) == false );
    REQUIRE( p->all_tokens->tokens[0]->error == true );
    json_parser_cleanup(p);
}

TEST_CASE( "json_parsenum_multiple_e", "[json_parsenum]")
{
    char *err_str = "123E10E10";
    json_parser *p = json_parser_create(err_str);
    REQUIRE( json_parsenum(p, p->all_tokens->tokens[0]) == false );
    REQUIRE( p->all_tokens->tokens[0]->error == true );
    json_parser_cleanup(p);
}

TEST_CASE( "json_parsebool_true", "[json_parsebool]" )
{
    char *bool_str = "true ";
    json_parser *p = json_parser_create(bool_str);
    REQUIRE( json_parsebool(p, p->all_tokens->tokens[0]) == true );
    json_jsontoken *t = p->all_tokens->tokens[1];
    REQUIRE( t->type == JSON_BOO );
    REQUIRE( t->start_in == 0 );
    REQUIRE( t->end_in == 4 );
    json_parser_cleanup(p);
}

TEST_CASE( "json_parsebool_false", "[json_parsebool]" )
{
    char *bool_str = "false ";
    json_parser *p = json_parser_create(bool_str);
    REQUIRE( json_parsebool(p, p->all_tokens->tokens[0]) == true );
    json_jsontoken *t = p->all_tokens->tokens[1];
    REQUIRE( t->type == JSON_BOO );
    REQUIRE( t->start_in == 0 );
    REQUIRE( t->end_in == 5 );
    json_parser_cleanup(p);
}

TEST_CASE( "json_parsebool_err", "[json_parsebool]" )
{
    char *err_str = "tRue ";
    json_parser *p = json_parser_create(err_str);
    REQUIRE( json_parsebool(p, p->all_tokens->tokens[0]) == false );
    REQUIRE( p->all_tokens->tokens[0]->error == true );
    json_parser_cleanup(p);
}

TEST_CASE( "json_parsearr_one", "[json_parsearr]" )
{
    char *arr_str = "[ \"hello\", 1234, 12, \
    null, true, 12.12e100, [], {} \
] \
";
    json_parser *p = json_parser_create(arr_str);
    REQUIRE( json_parsearr(p, p->all_tokens->tokens[0]) == true );
    json_jsontoken *t = p->all_tokens->tokens[1];
    REQUIRE( t->children->length == 8 );
    int i = 0;
    REQUIRE( t->children->tokens[i]->type == JSON_STR );
    REQUIRE( t->children->tokens[i]->start_in == 3 );
    REQUIRE( t->children->tokens[i++]->end_in == 8 );
    REQUIRE( t->children->tokens[i]->type == JSON_INT );
    REQUIRE( t->children->tokens[i]->start_in == 11 );
    REQUIRE( t->children->tokens[i++]->end_in == 15 );
    REQUIRE( t->children->tokens[i]->type == JSON_INT );
    REQUIRE( t->children->tokens[i]->start_in == 17 );
    REQUIRE( t->children->tokens[i++]->end_in == 19 );
    REQUIRE( t->children->tokens[i]->type == JSON_NUL );
    REQUIRE( t->children->tokens[i]->start_in == 25 );
    REQUIRE( t->children->tokens[i++]->end_in == 29 );
    REQUIRE( t->children->tokens[i]->type == JSON_BOO );
    REQUIRE( t->children->tokens[i]->start_in == 31 );
    REQUIRE( t->children->tokens[i++]->end_in == 35 );
    REQUIRE( t->children->tokens[i]->type == JSON_FLO );
    REQUIRE( t->children->tokens[i]->start_in == 37 );
    REQUIRE( t->children->tokens[i++]->end_in == 46 );
    REQUIRE( t->children->tokens[i]->type == JSON_ARR );
    REQUIRE( t->children->tokens[i]->start_in == 48 );
    REQUIRE( t->children->tokens[i++]->end_in == 50 );
    REQUIRE( t->children->tokens[i]->type == JSON_OBJ );
    REQUIRE( t->children->tokens[i]->start_in == 52 );
    REQUIRE( t->children->tokens[i++]->end_in == 54 );
    REQUIRE( t->start_in == 0 );
    REQUIRE( t->end_in == 56 );
    json_parser_cleanup(p);
}
