#include "cjson.h"

#include <stdlib.h>
#include <stdio.h>

int main() {
    bool is_one = false;
    char *test = "{ \"hello\": [\n\tnull,\n\t\"Hello world\", {\"hi\": null} ], \"world\": 1234.5E100, \"world\": 1, \"world\": 1, \"world\": 1, \"world\": 1, \"world\": 1, \"world\": 1, \"world\": 1, \"world\": 1, \"world\": 1, \"world\": 1, \"world\": 1, \"world\": 1}";
    json_parser *parser = json_parser_create(test);
    bool res = json_parseobj(parser, parser->all_tokens->tokens[0]);
    printf("%c \n", parser->input[parser->curr-1]);
    printf("res: %d \n", res);
    printf("num tokens: %d \n", parser->all_tokens->length);
    json_parser_cleanup(parser);
}