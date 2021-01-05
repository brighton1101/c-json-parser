#define CATCH_CONFIG_MAIN
#include "ext/catch.hpp"

#include "../cjson.h"

TEST_CASE( "TEST", "[json_iswhitespace]" ){
    REQUIRE( json_iswhitespace(' ') == 1 );
}