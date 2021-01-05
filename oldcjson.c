#include <stdlib.h>
#include <stdio.h>

#define JSON_MAX_NESTED_DEPTH 1024

typedef enum {
    JSON_UND = 0,  // undefined (error)
    JSON_OBJ = 1,  // object
    JSON_ARR = 2,  // array
    JSON_STR = 3,  // string
    JSON_BOO = 4,  // boolean
    JSON_INT = 5,  // integer
    JSON_FLO = 6,  // float
} json_jsontoken_type;

typedef struct {
    json_jsontoken_type type;  // token type
    int start;                 // start of token
    int end;                   // end of token
    int num_children;          // number of children
} json_jsontoken;

typedef struct {
    char* input;
    int start;
    int curr;
    int end;
    int depth;
} json_parser;

/** reference: https://en.cppreference.com/w/cpp/string/byte/isspace */
int json_iswhitespace(char c) {
    switch (c) {
        case 0x20:
        case 0x0c:
        case 0x0a:
        case 0x0d:
        case 0x09:
        case 0x0b:
            return 1;
        default:
            return 0;
    }
}

typedef struct {
    char[JSON_MAX_NESTED_DEPTH] items;
    int start;
    int end;
} json_tokens_stack;

int
json_tokens_stack_push(json_tokens_stack *s, char c) {
    if (s->end+1 >= JSON_MAX_NESTED_DEPTH)
        return -1;
    s->items[s->end++] = c;
}

char
json_tokens_stack_pop(json_tokens_stack *s) {
    if (s->end == 0)
        return 0;
    return s->input[s->end--];
}

json_tokens_stack*
json_create_tokens_stack() {
    json_tokens_stack* s = (json_tokens_stack*) malloc(sizeof(json_tokens_stack));
    s->start = 0;
    s->end = 0;
    return s;
}

/*
['{', '\n', '\t', '"', 'a', '"', ':', ' ', '"', 'a', '"', '\n', '}']

{
    'a': 'a'
}

 */
int json_parsestr(json_parser *parser, json_jsontoken *parent) {
    char curr_c;
    unsigned prev_backslash;
    prev_backslash = 0;
    while (1) {
        curr_c = parser->input[parser->curr++];
        if (curr_c == '\0')
            return -1;
        else if (curr_c == '"' && !prev_backslash)
            return parser->curr - 1;
        else if (curr_c == '\\')
            prev_backslash = 2;
        else
            prev_backslash = 1;
        prev_backslash -= 1;
    }
}

int json_advancenumerical(json_parser *parser) {
    char curr_c;
    unsigned seen_e = 0;
    unsigned seen_dec = 0;
    while (1) {
        curr_c = parser->input[parser->curr++];
        if (curr_c == '\0')
            return -1;
        switch (curr_c) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                continue;
            default:
                if (curr_c == 'E' && seen_e > 0)
                    return -1;
                else if (curr_c == 'E')
                    seen_e = 1;
                else if (curr_c == '.' && seen_dec > 0)
                    return -1;
                else if (curr_c == '.')
                    seen_dec = 1;
                else if (json_iswhitespace(curr_c))
                    return parser->curr - 1;
                else
                    return -1;
        }
    }
}

int
json_isnumericalchar(char c) {
    switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return 1;
        default:
            return 0;
    }
}

/*
int
json_parseobject(json_parser *parser, json_jsontoken *parent)
{
    char curr_c;
    json_jsontoken *curr_obj;
    unsigned curr_is_key;
    
    curr_obj = (json_jsontoken*) malloc(sizeof(json_jsontoken));
    curr_obj->type = JSON_OBJ;
    curr_obj->parent = parent;
    curr_obj->start = parser->curr - 1;
    curr_is_key = 0;
    while (1) {
        curr_c = parser->input[parser->curr++];
        if (json_iswhitespace(curr_c))
            continue;
        else if (curr_c == '"')
            json_parsestr(parser,)
        if (json_isnumericalchar(c))
            json_parsenumeric(parser, curr_obj);
    }
}

int
json_parsenumeric(json_parser *parser, json_jsontoken *parent)
{

} */



int main() {
    json_parser *parser;
    parser = (json_parser*) malloc(sizeof(json_parser));
    parser->curr = 0;
    char* in = "ab\\\"cd\"";
    parser->input = in;
    int res;
    res = json_advancestr(parser);
    printf("%d\n", res);
    printf("%c\n", parser->input[res-1]);
    printf("%d\n", parser->curr);
}

